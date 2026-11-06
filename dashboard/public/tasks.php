<?php
    require_once '../src/bootstrap.php';

    $taskid = (int)($_GET['taskid'] ?? 0);

    $activityService = new ActivityService();
    $task = $taskid ? $activityService->getTaskDetails($taskid) : null;

    if (!$task) {
        http_response_code(404);
        die('Task not found.');
    }

    $activityid = $task['activityid'];

    if (!$activityService->isProjectTeamMember($activityid, $eid)) {
        http_response_code(403);
        die('Access denied: you are not a member of this activity\'s project team.');
    }

    $message = '';
    $messageType = '';

    $isActivityManager = $activityService->isActivityManager($activityid, $eid);

    // Task details can be edited by the project Manager, a project Lead, or the Activity manager.
    // The same roles can delete the task, reassign its open assignments, and assign it once it
    // has been closed (no open assignments).
    $canEditTask = $isActivityManager || $activityService->isProjectLeadOrManager($activityid, $eid);
    $canDeleteTask = $canEditTask;
    $canReassignTask = $canEditTask;
    $canAssignTask = $canEditTask;

    // Tasks can only be assigned or reassigned to members of the activity's project team
    $teamMembers = $activityService->getActivityTeamMembers($activityid);
    $teamMemberIds = array_map('intval', array_column($teamMembers, 'employeeid'));

    // CSRF protection for all data-mutating POST actions on this page
    $mutatingActions = ['edit_task', 'close_task', 'reassign_task', 'delete_task', 'assign_task'];
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && array_intersect(array_keys($_POST), $mutatingActions) && !Csrf::validate($_POST['csrf_token'] ?? '')) {
        http_response_code(403);
        die('Invalid or expired form submission (CSRF token mismatch). Please refresh the page and try again.');
    }

    // Handle task editing
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['edit_task'])) {
        $taskname = trim($_POST['edit_taskname'] ?? '');
        $taskdescription = trim($_POST['edit_taskdescription'] ?? '');
        $departmentid = $_POST['edit_departmentid'] ?? '';
        $taskdescription = $taskdescription !== '' ? $taskdescription : null;
        $departmentid = ctype_digit((string)$departmentid) && (int)$departmentid > 0 ? (int)$departmentid : null;
        $validDepartmentIds = array_column($activityService->getDepartments(), 'departmentid');

        if (!$canEditTask) {
            $message = 'Only the project manager, a project lead or the activity manager can edit task details.';
            $messageType = 'danger';
        } elseif ($taskname === '') {
            $message = 'Please enter a task name.';
            $messageType = 'warning';
        } elseif (mb_strlen($taskname) > 255 || ($taskdescription !== null && mb_strlen($taskdescription) > 2000)) {
            $message = 'Task name must be at most 255 characters and the description at most 2000 characters.';
            $messageType = 'warning';
        } elseif ($departmentid !== null && !in_array($departmentid, array_map('intval', $validDepartmentIds), true)) {
            $message = 'Please select a valid department.';
            $messageType = 'warning';
        } elseif ($activityService->updateTaskDetails($taskid, $taskname, $taskdescription, $departmentid)) {
            $message = 'Task updated successfully!';
            $messageType = 'success';
            $task = $activityService->getTaskDetails($taskid);
        } else {
            $message = 'Error updating task. Please try again.';
            $messageType = 'danger';
        }
    }

    // Handle task deletion: the Activity manager, project Leads and the project Manager can delete a task, but only if it has
    // no sub-tasks and no booked hours (Hours rows reference the task's assignments)
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['delete_task'])) {
        $hasSubtasks = false;
        foreach ($activityService->getTaskHeirarchy($activityid) as $t) {
            if ($t['parenttaskid'] == $taskid) {
                $hasSubtasks = true;
                break;
            }
        }
        $bookedHours = array_sum(array_column($activityService->getTaskAssignmentHistory($taskid), 'booked_hours'));

        if (!$canDeleteTask) {
            $message = 'Only the activity manager, a project lead or the project manager can delete tasks.';
            $messageType = 'danger';
        } elseif ($hasSubtasks) {
            $message = 'Tasks with sub-tasks cannot be deleted.';
            $messageType = 'warning';
        } elseif ($bookedHours > 0) {
            $message = 'Tasks with booked hours cannot be deleted.';
            $messageType = 'warning';
        } elseif ($activityService->deleteActivityTask($taskid)) {
            header('Location: activity.php?activityid=' . urlencode($activityid));
            exit;
        } else {
            $message = 'Error deleting task. Please try again.';
            $messageType = 'danger';
        }
    }

    // Returns the current user's open assignment on this task, or null if they have none
    function findOwnOpenAssignment($assignments, $eid) {
        foreach ($assignments as $assignment) {
            if ($assignment['status'] === 'Open' && $assignment['userid'] == $eid) {
                return $assignment;
            }
        }
        return null;
    }

    // Handle task closing:
    //  - the Activity manager closes the task, closing every open assignment on it
    //  - any other user with an open assignment completes it and hands the task back to the Activity manager
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['close_task'])) {
        $openAssignments = array_filter($activityService->getTaskAssignmentHistory($taskid), fn($a) => $a['status'] === 'Open');
        $ownAssignment = findOwnOpenAssignment($openAssignments, $eid);
        $closedate = date('Y-m-d');

        if (!$openAssignments) {
            $message = 'This task has no open assignments to close.';
            $messageType = 'warning';
        } elseif ($isActivityManager) {
            if ($activityService->closeAllTaskAssignments($taskid, $closedate) !== false) {
                $message = 'Task closed successfully.';
                $messageType = 'success';
            } else {
                $message = 'Error closing task. Please try again.';
                $messageType = 'danger';
            }
        } elseif (!$ownAssignment) {
            $message = 'You do not have an open assignment on this task.';
            $messageType = 'danger';
        } else {
            $result = $activityService->closeTask($ownAssignment['assignmentid'], $eid, $activityid, $closedate);
            $message = $result['message'] . '.';
            $messageType = $result['success'] ? 'success' : 'danger';
        }
    }

    // Handle task reassignment: the Activity manager, project Leads and the project Manager can reassign any open assignment
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['reassign_task'])) {
        $assignmentid = $_POST['reassign_assignmentid'] ?? '';
        $newuserid = $_POST['reassign_userid'] ?? '';
        $assignment = null;
        foreach ($activityService->getTaskAssignmentHistory($taskid) as $a) {
            if ($a['status'] === 'Open' && $a['assignmentid'] == $assignmentid) {
                $assignment = $a;
                break;
            }
        }
        if (!$canReassignTask) {
            $message = 'Only the activity manager, a project lead or the project manager can reassign tasks.';
            $messageType = 'danger';
        } elseif (!$assignment) {
            $message = 'Invalid assignment selection.';
            $messageType = 'warning';
        } elseif (!ctype_digit((string)$newuserid) || !in_array((int)$newuserid, $teamMemberIds, true)) {
            $message = 'Please select a member of this project team to reassign the task to.';
            $messageType = 'warning';
        } elseif ($activityService->reassignTask($assignment['assignmentid'], (int)$newuserid, date('Y-m-d'))) {
            $message = 'Task reassigned successfully!';
            $messageType = 'success';
        } else {
            $message = 'Error reassigning task. The selected user may already be assigned to the task.';
            $messageType = 'danger';
        }
    }

    // Handle task assignment: the Activity manager, project Leads and the project Manager
    // can assign the task to a user once every previous assignment has been closed
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['assign_task'])) {
        $userid = $_POST['assign_userid'] ?? '';
        $hasOpenAssignment = (bool)array_filter($activityService->getTaskAssignmentHistory($taskid), fn($a) => $a['status'] === 'Open');

        if (!$canAssignTask) {
            $message = 'Only the project manager, a project lead or the activity manager can assign tasks.';
            $messageType = 'danger';
        } elseif ($hasOpenAssignment) {
            $message = 'This task already has an open assignment. Use Reassign Task instead.';
            $messageType = 'warning';
        } elseif (!ctype_digit((string)$userid) || !in_array((int)$userid, $teamMemberIds, true)) {
            $message = 'Please select a member of this project team to assign the task to.';
            $messageType = 'warning';
        } elseif ($activityService->assignTask($taskid, (int)$userid, date('Y-m-d'))) {
            $message = 'Task assigned successfully!';
            $messageType = 'success';
        } else {
            $message = 'Error assigning task. Please try again.';
            $messageType = 'danger';
        }
    }

    $departments = $canEditTask ? $activityService->getDepartments() : [];

    $activitydata = $activityService->getActivityDetails($activityid);
    $assignments = $activityService->getTaskAssignmentHistory($taskid);

    // Status and hours roll up from sub-tasks, so take them from the activity's task hierarchy
    $taskHeirarchy = $activityService->getTaskHeirarchy($activityid);
    $statusLabels = [
        TaskStatus::COMPLETE => 'Complete',
        TaskStatus::IN_PROGRESS => 'In progress',
        TaskStatus::NOT_STARTED => 'Not started',
    ];
    $taskStatus = $statusLabels[$taskHeirarchy[$taskid]['status']] ?? 'Not started';
    $totalHours = $taskHeirarchy[$taskid]['hours'] ?? 0;

    $subtasks = [];
    foreach ($taskHeirarchy as $t) {
        if ($t['parenttaskid'] == $taskid) {
            $subtasks[] = $t;
        }
    }

    $currentAssignees = [];
    foreach ($assignments as $assignment) {
        if ($assignment['status'] === 'Open') {
            $currentAssignees[] = $assignment['assignee'];
        }
    }

    // The Activity manager can close the task while it has open assignments;
    // anyone else can complete their own open assignment
    $hasOwnOpenAssignment = findOwnOpenAssignment($assignments, $eid) !== null;
    $canCloseTask = $isActivityManager ? !empty($currentAssignees) : $hasOwnOpenAssignment;

    // Open assignments the current user may reassign
    $reassignableAssignments = [];
    $openAssigneeIds = [];
    foreach ($assignments as $assignment) {
        if ($assignment['status'] !== 'Open') continue;
        $openAssigneeIds[] = (int)$assignment['userid'];
        if ($canReassignTask) {
            $reassignableAssignments[] = $assignment;
        }
    }
    // Reassign is only offered while the task has open assignments
    $canReassignTask = !empty($reassignableAssignments);
    // Assigning is only enabled while the task has no open assignments
    $isTaskClosed = empty($openAssigneeIds);
    $assignCandidates = ($canAssignTask && $isTaskClosed) ? $teamMembers : [];
    // Anyone already holding an open assignment on this task can't be given another one
    $reassignCandidates = $canReassignTask
        ? array_filter($teamMembers, fn($e) => !in_array((int)$e['employeeid'], $openAssigneeIds, true))
        : [];

    $pageTitle = $task['taskname'];
    include 'includes/head.php';
?>
<body>
    <?php include 'includes/header.php'; ?>
    <main class="container app-main">
        <div class="page-header">
            <div>
                <nav aria-label="breadcrumb">
                    <ol class="breadcrumb">
                        <li class="breadcrumb-item"><a href="dashboard.php">Dashboard</a></li>
                        <li class="breadcrumb-item"><a href="activity.php?activityid=<?php echo urlencode($activityid); ?>"><?php echo htmlspecialchars($activitydata['activity_name']); ?></a></li>
                        <li class="breadcrumb-item active" aria-current="page"><?php echo htmlspecialchars($task['taskname']); ?></li>
                    </ol>
                </nav>
                <div class="section-label">Task</div>
                <h1 class="h3"><?php echo htmlspecialchars($task['taskname']); ?></h1>
                <p class="page-subtitle">
                    <?php echo htmlspecialchars($activitydata['project_number'] . ' ' . $activitydata['project_name']); ?>
                    &middot; <?php echo htmlspecialchars($activitydata['activity_name']); ?>
                </p>
            </div>
        </div>

        <?php if ($message): ?>
            <div class="alert alert-<?php echo $messageType; ?> alert-dismissible fade show" role="alert">
                <?php echo htmlspecialchars($message); ?>
                <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Close"></button>
            </div>
        <?php endif; ?>

        <!-- Task Details -->
        <div class="card mb-4">
            <div class="card-header">
                <h2 class="card-title">Task Details</h2>
                <?php if ($canEditTask || $canDeleteTask): ?>
                    <div class="card-actions">
                        <?php if ($canEditTask): ?>
                            <button type="button" class="btn btn-sm btn-outline-secondary" data-bs-toggle="modal" data-bs-target="#editTaskModal">
                                <i class="bi bi-pencil"></i> Edit Task
                            </button>
                        <?php endif; ?>
                        <?php if ($canDeleteTask): ?>
                            <button type="button" class="btn btn-sm btn-outline-danger" data-bs-toggle="modal" data-bs-target="#deleteTaskModal">
                                <i class="bi bi-trash"></i> Delete Task
                            </button>
                        <?php endif; ?>
                    </div>
                <?php endif; ?>
            </div>
            <div class="card-body">
                <?php if (!empty($task['taskdescription'])): ?>
                    <p><?php echo htmlspecialchars($task['taskdescription']); ?></p>
                <?php endif; ?>
                <dl class="row detail-list">
                    <dt class="col-sm-3">Path</dt>
                    <dd class="col-sm-9"><?php echo htmlspecialchars($task['task_hierarchy']); ?></dd>
                    <?php if ($task['parenttaskid']): ?>
                        <dt class="col-sm-3">Parent task</dt>
                        <dd class="col-sm-9">
                            <a href="tasks.php?taskid=<?php echo urlencode($task['parenttaskid']); ?>"><?php echo htmlspecialchars($taskHeirarchy[$task['parenttaskid']]['taskname'] ?? ''); ?></a>
                        </dd>
                    <?php endif; ?>
                    <dt class="col-sm-3">Status</dt>
                    <dd class="col-sm-9"><?php echo StatusBadge::html($taskStatus); ?></dd>
                    <dt class="col-sm-3">Assigned to</dt>
                    <dd class="col-sm-9"><?php echo htmlspecialchars($currentAssignees ? implode(', ', $currentAssignees) : '-'); ?></dd>
                    <dt class="col-sm-3">Department</dt>
                    <dd class="col-sm-9"><?php echo htmlspecialchars($task['departmentname'] ?? '-'); ?></dd>
                    <dt class="col-sm-3">Booked hours</dt>
                    <dd class="col-sm-9">
                        <?php echo htmlspecialchars($totalHours); ?> h
                        <?php if (!empty($subtasks)): ?>
                            <span class="text-body-secondary">(including sub-tasks)</span>
                        <?php endif; ?>
                    </dd>
                </dl>
            </div>
        </div>

        <!-- Assignments -->
        <div class="card mb-4">
            <div class="card-header">
                <h2 class="card-title">Assignments</h2>
                <?php if ($canAssignTask || $canCloseTask || $canReassignTask): ?>
                    <div class="card-actions">
                        <?php if ($canAssignTask): ?>
                            <?php if ($isTaskClosed): ?>
                                <button type="button" class="btn btn-sm btn-outline-secondary" data-bs-toggle="modal" data-bs-target="#assignTaskModal">
                                    <i class="bi bi-person-plus"></i> Assign Task
                                </button>
                            <?php else: ?>
                                <span class="d-inline-block" tabindex="0" title="The task can be assigned once it has been closed">
                                    <button type="button" class="btn btn-sm btn-outline-secondary" disabled>
                                        <i class="bi bi-person-plus"></i> Assign Task
                                    </button>
                                </span>
                            <?php endif; ?>
                        <?php endif; ?>
                        <?php if ($canReassignTask): ?>
                            <button type="button" class="btn btn-sm btn-outline-secondary" data-bs-toggle="modal" data-bs-target="#reassignTaskModal">
                                <i class="bi bi-arrow-left-right"></i> Reassign Task
                            </button>
                        <?php endif; ?>
                        <?php if ($canCloseTask): ?>
                            <button type="button" class="btn btn-sm btn-outline-secondary" data-bs-toggle="modal" data-bs-target="#closeTaskModal">
                                <?php if ($isActivityManager): ?>
                                    <i class="bi bi-check2-circle"></i> Close Task
                                <?php else: ?>
                                    <i class="bi bi-send-check"></i> Complete Task
                                <?php endif; ?>
                            </button>
                        <?php endif; ?>
                    </div>
                <?php endif; ?>
            </div>
            <?php if (empty($assignments)): ?>
                <div class="empty-state">
                    <i class="bi bi-person-dash"></i>
                    This task has not been assigned to anyone.
                </div>
            <?php else: ?>
                <div class="table-responsive">
                    <table class="table table-hover align-middle mb-0">
                        <thead>
                            <tr>
                                <th>Assignee</th>
                                <th>Status</th>
                                <th>Assigned</th>
                                <th>Closed</th>
                                <th class="text-end">Booked hours</th>
                            </tr>
                        </thead>
                        <tbody>
                            <?php foreach ($assignments as $assignment): ?>
                                <tr>
                                    <td class="fw-medium"><?php echo htmlspecialchars($assignment['assignee']); ?></td>
                                    <td><?php echo StatusBadge::html($assignment['status']); ?></td>
                                    <td><?php echo htmlspecialchars($assignment['assigneddate'] ?? '-'); ?></td>
                                    <td><?php echo htmlspecialchars($assignment['closedate'] ?? '-'); ?></td>
                                    <td class="text-end"><?php echo htmlspecialchars($assignment['booked_hours']); ?></td>
                                </tr>
                            <?php endforeach; ?>
                        </tbody>
                    </table>
                </div>
            <?php endif; ?>
        </div>

        <?php if (!empty($subtasks)): ?>
            <!-- Sub-tasks -->
            <div class="card mb-4">
                <div class="card-header">
                    <h2 class="card-title">Sub-tasks</h2>
                </div>
                <div class="table-responsive">
                    <table class="table table-hover align-middle mb-0">
                        <thead>
                            <tr>
                                <th>Task</th>
                                <th>Status</th>
                                <th class="text-end">Booked hours</th>
                            </tr>
                        </thead>
                        <tbody>
                            <?php foreach ($subtasks as $subtask): ?>
                                <tr>
                                    <td class="fw-medium">
                                        <a href="tasks.php?taskid=<?php echo urlencode($subtask['taskid']); ?>" class="text-reset text-decoration-none">
                                            <?php echo htmlspecialchars($subtask['taskname']); ?>
                                        </a>
                                    </td>
                                    <td><?php echo StatusBadge::html($statusLabels[$subtask['status']] ?? 'Not started'); ?></td>
                                    <td class="text-end"><?php echo htmlspecialchars($subtask['hours']); ?></td>
                                </tr>
                            <?php endforeach; ?>
                        </tbody>
                    </table>
                </div>
            </div>
        <?php endif; ?>
    </main>

    <?php if ($canEditTask): ?>
        <!-- Edit Task Modal -->
        <div class="modal fade" id="editTaskModal" tabindex="-1" aria-labelledby="editTaskModalLabel" aria-hidden="true">
            <div class="modal-dialog">
                <div class="modal-content">
                    <form method="POST">
                        <?php echo Csrf::field(); ?>
                        <div class="modal-header">
                            <h5 class="modal-title" id="editTaskModalLabel">Edit Task Details</h5>
                            <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                        </div>
                        <div class="modal-body">
                            <div class="mb-3">
                                <label for="edit_taskname" class="form-label">Task name</label>
                                <input type="text" class="form-control" id="edit_taskname" name="edit_taskname" maxlength="255"
                                       value="<?php echo htmlspecialchars($task['taskname']); ?>" required>
                            </div>
                            <div class="mb-3">
                                <label for="edit_taskdescription" class="form-label">Description</label>
                                <textarea class="form-control" id="edit_taskdescription" name="edit_taskdescription" rows="3" maxlength="2000"><?php echo htmlspecialchars($task['taskdescription'] ?? ''); ?></textarea>
                            </div>
                            <div>
                                <label for="edit_departmentid" class="form-label">Department</label>
                                <select class="form-select" id="edit_departmentid" name="edit_departmentid">
                                    <option value="">None</option>
                                    <?php foreach ($departments as $department): ?>
                                        <option value="<?php echo (int)$department['departmentid']; ?>"<?php echo (int)$department['departmentid'] === (int)$task['departmentid'] ? ' selected' : ''; ?>>
                                            <?php echo htmlspecialchars($department['departmentname']); ?>
                                        </option>
                                    <?php endforeach; ?>
                                </select>
                            </div>
                        </div>
                        <div class="modal-footer">
                            <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                            <button type="submit" name="edit_task" class="btn btn-primary">Save Changes</button>
                        </div>
                    </form>
                </div>
            </div>
        </div>
    <?php endif; ?>

    <?php if ($canDeleteTask): ?>
        <!-- Delete Task Modal -->
        <div class="modal fade" id="deleteTaskModal" tabindex="-1" aria-labelledby="deleteTaskModalLabel" aria-hidden="true">
            <div class="modal-dialog">
                <div class="modal-content">
                    <form method="POST">
                        <?php echo Csrf::field(); ?>
                        <div class="modal-header">
                            <h5 class="modal-title" id="deleteTaskModalLabel">Delete Task</h5>
                            <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                        </div>
                        <div class="modal-body">
                            <p class="mb-3"><span class="text-body-secondary">Task:</span> <strong><?php echo htmlspecialchars($task['taskname']); ?></strong></p>
                            <?php if (!empty($subtasks)): ?>
                                <div class="alert alert-warning mb-0">
                                    <i class="bi bi-exclamation-triangle me-1"></i>
                                    This task has sub-tasks and cannot be deleted. Delete its sub-tasks first.
                                </div>
                            <?php elseif ($totalHours > 0): ?>
                                <div class="alert alert-warning mb-0">
                                    <i class="bi bi-exclamation-triangle me-1"></i>
                                    This task has booked hours and cannot be deleted.
                                </div>
                            <?php else: ?>
                                <div class="alert alert-danger mb-0">
                                    <i class="bi bi-exclamation-triangle me-1"></i>
                                    This will permanently delete the task and its assignment history. This action cannot be undone.
                                </div>
                            <?php endif; ?>
                        </div>
                        <div class="modal-footer">
                            <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                            <?php if (empty($subtasks) && $totalHours <= 0): ?>
                                <button type="submit" name="delete_task" class="btn btn-danger">Delete Task</button>
                            <?php endif; ?>
                        </div>
                    </form>
                </div>
            </div>
        </div>
    <?php endif; ?>

    <?php if ($canAssignTask && $isTaskClosed): ?>
        <!-- Assign Task Modal -->
        <div class="modal fade" id="assignTaskModal" tabindex="-1" aria-labelledby="assignTaskModalLabel" aria-hidden="true">
            <div class="modal-dialog">
                <div class="modal-content">
                    <form method="POST">
                        <?php echo Csrf::field(); ?>
                        <div class="modal-header">
                            <h5 class="modal-title" id="assignTaskModalLabel">Assign Task</h5>
                            <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                        </div>
                        <div class="modal-body">
                            <p class="mb-3"><span class="text-body-secondary">Task:</span> <strong><?php echo htmlspecialchars($task['taskname']); ?></strong></p>
                            <div class="mb-3">
                                <label for="assignUserSelect" class="form-label">Assign to</label>
                                <select class="form-select" id="assignUserSelect" name="assign_userid" required>
                                    <option value="">Please select a user...</option>
                                    <?php foreach ($assignCandidates as $employee): ?>
                                        <option value="<?php echo (int)$employee['employeeid']; ?>">
                                            <?php echo htmlspecialchars($employee['fullname']); ?>
                                        </option>
                                    <?php endforeach; ?>
                                </select>
                            </div>
                            <div>
                                <label for="assigndate" class="form-label">Assignment date</label>
                                <input type="date" class="form-control" id="assigndate"
                                       value="<?php echo date('Y-m-d'); ?>" readonly>
                            </div>
                        </div>
                        <div class="modal-footer">
                            <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                            <button type="submit" name="assign_task" class="btn btn-primary">Assign</button>
                        </div>
                    </form>
                </div>
            </div>
        </div>
    <?php endif; ?>

    <?php if ($canReassignTask): ?>
        <!-- Reassign Task Modal -->
        <div class="modal fade" id="reassignTaskModal" tabindex="-1" aria-labelledby="reassignTaskModalLabel" aria-hidden="true">
            <div class="modal-dialog">
                <div class="modal-content">
                    <form method="POST">
                        <?php echo Csrf::field(); ?>
                        <div class="modal-header">
                            <h5 class="modal-title" id="reassignTaskModalLabel">Reassign Task</h5>
                            <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                        </div>
                        <div class="modal-body">
                            <p class="mb-3"><span class="text-body-secondary">Task:</span> <strong><?php echo htmlspecialchars($task['taskname']); ?></strong></p>
                            <?php if (count($reassignableAssignments) === 1): ?>
                                <input type="hidden" name="reassign_assignmentid" value="<?php echo (int)$reassignableAssignments[0]['assignmentid']; ?>">
                                <p class="mb-3"><span class="text-body-secondary">Current assignee:</span> <?php echo htmlspecialchars($reassignableAssignments[0]['assignee']); ?></p>
                            <?php else: ?>
                                <div class="mb-3">
                                    <label for="reassignAssignmentSelect" class="form-label">Reassign from</label>
                                    <select class="form-select" id="reassignAssignmentSelect" name="reassign_assignmentid" required>
                                        <?php foreach ($reassignableAssignments as $assignment): ?>
                                            <option value="<?php echo (int)$assignment['assignmentid']; ?>"<?php echo $assignment['userid'] == $eid ? ' selected' : ''; ?>>
                                                <?php echo htmlspecialchars($assignment['assignee']); ?>
                                            </option>
                                        <?php endforeach; ?>
                                    </select>
                                </div>
                            <?php endif; ?>
                            <div class="mb-3">
                                <label for="reassignUserSelect" class="form-label">Reassign to</label>
                                <select class="form-select" id="reassignUserSelect" name="reassign_userid" required>
                                    <option value="">Please select a user...</option>
                                    <?php foreach ($reassignCandidates as $employee): ?>
                                        <option value="<?php echo (int)$employee['employeeid']; ?>">
                                            <?php echo htmlspecialchars($employee['fullname']); ?>
                                        </option>
                                    <?php endforeach; ?>
                                </select>
                            </div>
                            <div>
                                <label for="reassigndate" class="form-label">Reassignment date</label>
                                <input type="date" class="form-control" id="reassigndate"
                                       value="<?php echo date('Y-m-d'); ?>" readonly>
                            </div>
                        </div>
                        <div class="modal-footer">
                            <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                            <button type="submit" name="reassign_task" class="btn btn-primary">Reassign</button>
                        </div>
                    </form>
                </div>
            </div>
        </div>
    <?php endif; ?>

    <?php if ($canCloseTask): ?>
        <!-- Close Task Modal -->
        <div class="modal fade" id="closeTaskModal" tabindex="-1" aria-labelledby="closeTaskModalLabel" aria-hidden="true">
            <div class="modal-dialog">
                <div class="modal-content">
                    <form method="POST">
                        <?php echo Csrf::field(); ?>
                        <div class="modal-header">
                            <h5 class="modal-title" id="closeTaskModalLabel">
                                <?php echo $isActivityManager ? 'Close Task' : 'Complete Task'; ?>
                            </h5>
                            <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                        </div>
                        <div class="modal-body">
                            <p class="mb-3"><span class="text-body-secondary">Task:</span> <strong><?php echo htmlspecialchars($task['taskname']); ?></strong></p>
                            <div class="alert <?php echo $isActivityManager ? 'alert-success' : 'alert-warning'; ?>">
                                <?php if ($isActivityManager): ?>
                                    This will close every open assignment on this task (<?php echo htmlspecialchars(implode(', ', $currentAssignees)); ?>) and mark the task as completed.
                                <?php else: ?>
                                    This will complete your assignment and reassign the task to the activity manager.
                                <?php endif; ?>
                            </div>
                            <label for="closedate" class="form-label">Date</label>
                            <input type="date" class="form-control" id="closedate"
                                   value="<?php echo date('Y-m-d'); ?>" readonly>
                        </div>
                        <div class="modal-footer">
                            <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                            <button type="submit" name="close_task" class="btn <?php echo $isActivityManager ? 'btn-success' : 'btn-primary'; ?>">
                                <?php echo $isActivityManager ? 'Close' : 'Complete'; ?>
                            </button>
                        </div>
                    </form>
                </div>
            </div>
        </div>
    <?php endif; ?>

    <?php include 'includes/footer.php'; ?>
</body>
</html>
