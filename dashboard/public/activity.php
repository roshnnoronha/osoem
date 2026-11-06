<?php
    require_once '../src/bootstrap.php';

    if (isset($_GET['activityid']))
        $activityid = $_GET['activityid'];

    $activityService = new ActivityService();
    $artefactService = new ArtefactService();
    $milestoneService = new MilestoneService();

    if (!$activityService->isProjectTeamMember($activityid, $eid)) {
        http_response_code(403);
        die('Access denied: you are not a member of this activity\'s project team.');
    }

    $isActivityManager = $activityService->isActivityManager($activityid, $eid);
    $isProjectLeadOrManager = $activityService->isProjectLeadOrManager($activityid, $eid);

    // The activity manager, project Leads and the project Manager can edit the activity, manage its
    // tasks, link/unlink artefacts and set/mark milestones. Only Leads and the Manager can delegate it.
    $canManageActivity = $isActivityManager || $isProjectLeadOrManager;
    $canDelegateActivity = $isProjectLeadOrManager;

    // Task ids in this activity, used to reject task ids posted from other activities
    function activityTaskIds($activityService, $activityid) {
        return array_map('intval', array_column($activityService->getTasksForParentSelection($activityid), 'taskid'));
    }

    $message = '';
    $messageType = '';

    // CSRF protection for all data-mutating POST actions on this page
    $mutatingActions = ['add_note', 'delete_note', 'edit_note', 'add_task', 'assign_task', 'edit_activity', 'delegate_activity', 'reassign_open_task', 'close_task', 'set_milestones', 'link_artefact', 'unlink_artefact', 'associate_milestone'];
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && array_intersect(array_keys($_POST), $mutatingActions) && !Csrf::validate($_POST['csrf_token'] ?? '')) {
        http_response_code(403);
        die('Invalid or expired form submission (CSRF token mismatch). Please refresh the page and try again.');
    }

    // Handle note submission
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['add_note'])) {
        $note = trim($_POST['note'] ?? '');
        $notedate = date('Y-m-d');
        
        if (!empty($note)) {
            if ($activityService->addActivityNote($activityid, $eid, $note, $notedate)) {
                $message = 'Note added successfully!';
                $messageType = 'success';
            } else {
                $message = 'Error adding note. Please try again.';
                $messageType = 'danger';
            }
        } else {
            $message = 'Please enter a note.';
            $messageType = 'warning';
        }
    }
    
    // Handle note deletion
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['delete_note'])) {
        $noteid = $_POST['noteid'] ?? '';
        
        if ($noteid) {
            if ($activityService->deleteActivityNote($noteid, $eid)) {
                $message = 'Note deleted successfully!';
                $messageType = 'success';
            } else {
                $message = 'Error deleting note. You can only delete your own notes.';
                $messageType = 'danger';
            }
        }
    }
    
    // Handle note editing
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['edit_note'])) {
        $noteid = $_POST['noteid'] ?? '';
        $note = trim($_POST['note'] ?? '');

        if ($noteid && !empty($note)) {
            if ($activityService->updateActivityNote($noteid, $eid, $note)) {
                $message = 'Note updated successfully!';
                $messageType = 'success';
            } else {
                $message = 'Error updating note. You can only edit your own notes.';
                $messageType = 'danger';
            }
        } else {
            $message = 'Please fill in all fields.';
            $messageType = 'warning';
        }
    }

    // Handle task addition
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['add_task'])) {
        $taskname = trim($_POST['taskname'] ?? '');
        $parenttaskid = (int)($_POST['parenttaskid'] ?? 0) ?: null;

        if (!$canManageActivity) {
            $message = 'Only the activity manager, a project lead or the project manager can add tasks.';
            $messageType = 'danger';
        } elseif ($parenttaskid !== null && !in_array($parenttaskid, activityTaskIds($activityService, $activityid), true)) {
            $message = 'Please select a parent task in this activity.';
            $messageType = 'warning';
        } elseif (!empty($taskname)) {
            if ($activityService->addActivityTask($activityid, $taskname, $parenttaskid)) {
                $message = 'Task added successfully!';
                $messageType = 'success';
            } else {
                $message = 'Error adding task. Please try again.';
                $messageType = 'danger';
            }
        } else {
            $message = 'Please enter a task name.';
            $messageType = 'warning';
        }
    }

    // Handle task assignment
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['assign_task'])) {
        $taskid = (int)($_POST['assign_taskid'] ?? 0);
        $userid = $_POST['assign_userid'] ?? '';
        $assigneddate = date('Y-m-d');

        if (!$canManageActivity) {
            $message = 'Only the activity manager, a project lead or the project manager can assign tasks.';
            $messageType = 'danger';
        } elseif ($taskid && !in_array($taskid, activityTaskIds($activityService, $activityid), true)) {
            $message = 'Please select a task in this activity.';
            $messageType = 'warning';
        } elseif ($userid && !in_array((int)$userid, array_map('intval', array_column($activityService->getActivityTeamMembers($activityid), 'employeeid')), true)) {
            $message = 'Please select a member of this project team to assign the task to.';
            $messageType = 'warning';
        } elseif ($taskid && $userid) {
            if ($activityService->assignTask($taskid, $userid, $assigneddate)) {
                $message = 'Task assigned successfully!';
                $messageType = 'success';
            } else {
                $message = 'Error assigning task. Task may already be assigned.';
                $messageType = 'danger';
            }
        } else {
            $message = 'Please select both a task and a user for assignment.';
            $messageType = 'warning';
        }
    }

    // Handle activity editing
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['edit_activity'])) {
        $activityname = trim($_POST['edit_activityname'] ?? '');
        $activitydescription = trim($_POST['edit_activitydescription'] ?? '');
        $plannedstart = $_POST['edit_plannedstart'] ?? null;
        $plannedfinish = $_POST['edit_plannedfinish'] ?? null;
        $forecaststart = $_POST['edit_forecaststart'] ?? null;
        $forecastfinish = $_POST['edit_forecastfinish'] ?? null;
        $plannedhours = $_POST['edit_plannedhours'] ?? null;
        $forecasthours = $_POST['edit_forecasthours'] ?? null;

        // Convert empty strings to null for date fields
        $plannedstart = !empty($plannedstart) ? $plannedstart : null;
        $plannedfinish = !empty($plannedfinish) ? $plannedfinish : null;
        $forecaststart = !empty($forecaststart) ? $forecaststart : null;
        $forecastfinish = !empty($forecastfinish) ? $forecastfinish : null;
        $plannedhours = !empty($plannedhours) ? $plannedhours : null;
        $forecasthours = !empty($forecasthours) ? $forecasthours : null;

        if (!$canManageActivity) {
            $message = 'Only the activity manager, a project lead or the project manager can edit activity details.';
            $messageType = 'danger';
        } elseif (!empty($activityname)) {
            if ($activityService->updateActivity($activityid, $activityname, $activitydescription, $plannedstart, $plannedfinish, $forecaststart, $forecastfinish, $plannedhours, $forecasthours)) {
                $message = 'Activity updated successfully!';
                $messageType = 'success';
            } else {
                $message = 'Error updating activity. Please try again.';
                $messageType = 'danger';
            }
        } else {
            $message = 'Please enter an activity name.';
            $messageType = 'warning';
        }
    }

    // Handle changing the activity manager
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['delegate_activity'])) {
        $newmanagerid = $_POST['newmanagerid'] ?? '';
        $projectid = $activityService->getActivityDetails($activityid)['projectid'];
        $teamMemberIds = array_column((new ProjectService())->getProjectTeamMembers($projectid), 'employeeid');

        if (!$canDelegateActivity) {
            $message = 'Only a project lead or the project manager can change the activity manager.';
            $messageType = 'danger';
        } elseif (!in_array($newmanagerid, $teamMemberIds)) {
            $message = 'Please select a member of this project team.';
            $messageType = 'warning';
        } elseif ($activityService->updateActivityManager($activityid, (int)$newmanagerid)) {
            $message = 'Activity manager changed successfully!';
            $messageType = 'success';
        } else {
            $message = 'Error changing the activity manager. Please try again.';
            $messageType = 'danger';
        }
    }

    // Returns the submitted assignment ids that are the current user's open assignments in this activity
    function findOwnOpenAssignments($activityService, $activityid, $assignmentids, $eid) {
        $ownIds = array_map('intval', array_column($activityService->getActivityTasksForUser($activityid, $eid), 'assignmentid'));
        return array_values(array_intersect(array_unique(array_map('intval', (array)$assignmentids)), $ownIds));
    }

    // Handle reassigning any open task in the activity (Tasks card): limited to the
    // activity manager, project Leads and the project Manager
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['reassign_open_task'])) {
        $assignmentid = (int)($_POST['reassign_open_assignmentid'] ?? 0);
        $newuserid = $_POST['reassign_open_userid'] ?? '';
        $openAssignments = array_column($activityService->getOpenTaskAssignments($activityid), null, 'assignmentid');

        if (!$canManageActivity) {
            $message = 'Only the activity manager, a project lead or the project manager can reassign tasks.';
            $messageType = 'danger';
        } elseif (!isset($openAssignments[$assignmentid])) {
            $message = 'Invalid task selection.';
            $messageType = 'warning';
        } elseif (!ctype_digit((string)$newuserid) || !in_array((int)$newuserid, array_map('intval', array_column($activityService->getActivityTeamMembers($activityid), 'employeeid')), true)) {
            $message = 'Please select a member of this project team to reassign the task to.';
            $messageType = 'warning';
        } elseif ($activityService->reassignTask($assignmentid, (int)$newuserid, date('Y-m-d'))) {
            $message = 'Task reassigned successfully!';
            $messageType = 'success';
        } else {
            $message = 'Error reassigning task. The selected user may already be assigned to the task.';
            $messageType = 'danger';
        }
    }

    // Handle task closing (applies to every selected task)
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['close_task'])) {
        $assignmentids = findOwnOpenAssignments($activityService, $activityid, $_POST['assignmentids'] ?? [], $eid);
        $closedate = date('Y-m-d');

        if (!$assignmentids) {
            $message = 'Invalid task selection.';
            $messageType = 'warning';
        } else {
            $errors = [];
            $lastResult = null;
            foreach ($assignmentids as $assignmentid) {
                $result = $activityService->closeTask($assignmentid, $eid, $activityid, $closedate);
                if ($result['success']) {
                    $lastResult = $result;
                } else {
                    $errors[] = $result['message'];
                }
            }
            if (!$errors) {
                $message = count($assignmentids) === 1
                    ? $lastResult['message']
                    : count($assignmentids) . ' tasks ' . ($lastResult['action'] === 'closed' ? 'closed' : 'completed and handed back') . ' successfully.';
                $messageType = 'success';
            } else {
                $message = count($errors) === count($assignmentids) && count($errors) === 1
                    ? $errors[0]
                    : 'Error processing ' . count($errors) . ' of ' . count($assignmentids) . ' tasks: ' . implode('; ', array_unique($errors));
                $messageType = 'danger';
            }
        }
    }

    // Returns the artefact linked to this activity by $artefactlinkid, otherwise null.
    function findLinkedArtefact($artefactService, $activityid, $artefactlinkid) {
        foreach ($artefactService->getArtefactsSummaryForActivity($activityid) as $artefact) {
            if ($artefact['artefactlinkid'] == $artefactlinkid) {
                return $artefact;
            }
        }
        return null;
    }

    // Resolves the posted artefactlinkids[] to artefacts linked to this activity.
    // Returns null if the selection is empty/invalid or contains any artefact not linked to this activity.
    function findLinkedArtefacts($artefactService, $activityid, $artefactlinkids) {
        if (!is_array($artefactlinkids) || !$artefactlinkids) {
            return null;
        }
        $artefacts = [];
        foreach (array_unique($artefactlinkids) as $artefactlinkid) {
            $artefact = findLinkedArtefact($artefactService, $activityid, $artefactlinkid);
            if (!$artefact) {
                return null;
            }
            $artefacts[] = $artefact;
        }
        return $artefacts;
    }

    // Handle linking an existing project artefact to this activity
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['link_artefact'])) {
        $linkartefactid = $_POST['link_artefactid'] ?? '';
        $ratio = $_POST['link_ratio'] ?? '';
        $projectid = $activityService->getActivityDetails($activityid)['projectid'];
        $projectArtefactIds = array_column($artefactService->getArtefactsForProject($projectid), 'artefactid');

        if (!$canManageActivity) {
            $message = 'Only the activity manager, a project lead or the project manager can link artefacts.';
            $messageType = 'danger';
        } elseif (!in_array($linkartefactid, $projectArtefactIds) || !is_numeric($ratio) || $ratio <= 0 || $ratio > 1) {
            $message = 'Please select an artefact in this project and provide a ratio between 0 and 1.';
            $messageType = 'warning';
        } else {
            $result = $artefactService->linkArtefactToActivity($linkartefactid, $activityid, $ratio);
            $message = $result['message'];
            $messageType = $result['success'] ? 'success' : 'danger';
        }
    }

    // Handle removing artefact links from this activity (applies to every selected artefact)
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['unlink_artefact'])) {
        $artefacts = findLinkedArtefacts($artefactService, $activityid, $_POST['artefactlinkids'] ?? []);

        if (!$canManageActivity) {
            $message = 'Only the activity manager, a project lead or the project manager can remove linked artefacts.';
            $messageType = 'danger';
        } elseif (!$artefacts) {
            $message = 'Invalid artefact selection.';
            $messageType = 'warning';
        } else {
            $failed = 0;
            foreach ($artefacts as $artefact) {
                if (!$artefactService->unlinkArtefactFromActivity($artefact['artefactlinkid'])) {
                    $failed++;
                }
            }
            if ($failed === 0) {
                $message = count($artefacts) === 1 ? 'Artefact removed from this activity.' : count($artefacts) . ' artefacts removed from this activity.';
                $messageType = 'success';
            } else {
                $message = "Error removing $failed of " . count($artefacts) . ' artefacts. Please try again.';
                $messageType = 'danger';
            }
        }
    }

    // Handle associating a project milestone with the selected artefacts. Artefacts that already
    // have milestone steps are skipped, since an artefact link tracks progress against one milestone.
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['associate_milestone'])) {
        $milestoneid = (int)($_POST['milestoneid'] ?? 0);
        $planneddates = $_POST['planneddates'] ?? [];
        $projectid = $activityService->getActivityDetails($activityid)['projectid'];
        $projectMilestones = $milestoneService->getMilestonesForProject($projectid);
        $artefacts = findLinkedArtefacts($artefactService, $activityid, $_POST['artefactlinkids'] ?? []);

        if (!$canManageActivity) {
            $message = 'Only the activity manager, a project lead or the project manager can associate milestones with artefacts.';
            $messageType = 'danger';
        } elseif (!$artefacts) {
            $message = 'Invalid artefact selection.';
            $messageType = 'warning';
        } elseif (!isset($projectMilestones[$milestoneid]) || empty($projectMilestones[$milestoneid]['steps'])) {
            $message = 'Please select a milestone in this project that has steps.';
            $messageType = 'warning';
        } elseif (!is_array($planneddates)) {
            $message = 'Invalid planned dates.';
            $messageType = 'warning';
        } else {
            $planneddates = array_map(function ($date) { return $date !== '' ? $date : null; }, $planneddates);
            $associated = 0;
            $skipped = 0;
            $failed = 0;
            foreach ($artefacts as $artefact) {
                if ($milestoneService->getMilestoneSteps($artefact['artefactlinkid'])) {
                    $skipped++;
                } elseif ($milestoneService->associateMilestone($artefact['artefactlinkid'], $projectMilestones[$milestoneid]['steps'], $planneddates)) {
                    $associated++;
                } else {
                    $failed++;
                }
            }

            $parts = [];
            if ($associated) $parts[] = "Milestone associated with $associated artefact" . ($associated === 1 ? '' : 's') . '.';
            if ($skipped) $parts[] = "$skipped artefact" . ($skipped === 1 ? ' was' : 's were') . ' skipped because a milestone is already associated.';
            if ($failed) $parts[] = "Error associating $failed artefact" . ($failed === 1 ? '' : 's') . '. Please try again.';
            $message = implode(' ', $parts);
            $messageType = $failed ? 'danger' : ($skipped ? 'warning' : 'success');
        }
    }

    // Handle milestone step completion
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['set_milestones'])) {
        $artefactlinkid = $_POST['milestones_artefactlinkid'] ?? '';
        $completedlinkids = $_POST['completedlinkids'] ?? [];
        $completiondate = $_POST['milestonecompletiondate'] ?? date('Y-m-d');
        $artefact = findLinkedArtefact($artefactService, $activityid, $artefactlinkid);

        // Only project Leads and the project Manager can untick a completed step; for anyone else,
        // steps that are already complete stay complete regardless of what was submitted
        if ($artefact && is_array($completedlinkids) && !$isProjectLeadOrManager) {
            foreach ($milestoneService->getMilestoneSteps($artefact['artefactlinkid']) as $step) {
                if ($step['completiondate'] !== null) {
                    $completedlinkids[] = $step['milestonelinkid'];
                }
            }
            $completedlinkids = array_unique(array_map('intval', $completedlinkids));
        }

        // Any project team member can mark milestone steps (access to this page is limited to the team)
        if (!$artefact) {
            $message = 'Invalid artefact selection.';
            $messageType = 'warning';
        } elseif (!is_array($completedlinkids)) {
            $message = 'Invalid milestone selection.';
            $messageType = 'warning';
        } elseif ($milestoneService->setCompletedMilestoneSteps($artefact['artefactlinkid'], $completedlinkids, $eid, $completiondate)) {
            $message = 'Milestones updated successfully!';
            $messageType = 'success';
        } else {
            $message = 'Error updating milestones. Please try again.';
            $messageType = 'danger';
        }
    }

    $activitydata = $activityService->getActivityDetails($activityid);
    $activitydata['consumedhours'] = $activityService->getConsumedHours($activityid);
    $taskHeirarchy = $activityService->getTaskHeirarchy($activityid);
    $notelist = $activityService->getActivityNotes($activityid);
    $userTasks = $activityService->getActivityTasksForUser($activityid, $eid);

    // Re-evaluate after the handlers, since delegate_activity may have changed the activity manager
    $isActivityManager = $activityService->isActivityManager($activityid, $eid);
    $canManageActivity = $isActivityManager || $isProjectLeadOrManager;

    // Candidates for activity manager are limited to the project team
    $projectTeamMembers = $canDelegateActivity ? (new ProjectService())->getProjectTeamMembers($activitydata['projectid']) : [];
    $activityManagerId = $activityService->getActivityManagerId($activityid);

    // Get tasks for parent selection dropdown
    $tasksForParentSelection = $activityService->getTasksForParentSelection($activityid);

    // Get data for task assignment
    $unassignedTasks = $activityService->getUnassignedTasks($activityid);
    // Tasks and artefacts can only be assigned, reassigned or delegated to members of the project team
    $activityTeamMembers = $activityService->getActivityTeamMembers($activityid);

    $openTaskAssignments = $canManageActivity ? $activityService->getOpenTaskAssignments($activityid) : [];

    $artefactSummary = $artefactService->getArtefactsSummaryForActivity($activityid);
    foreach ($artefactSummary as &$artefact) {
        $artefact['milestonesteps'] = $milestoneService->getMilestoneSteps($artefact['artefactlinkid']);
    }
    unset($artefact);

    // Linking/removing artefacts is limited to the activity manager, project Leads and the project Manager
    $canLinkArtefacts = $canManageActivity;
    $linkableArtefacts = [];
    if ($canLinkArtefacts) {
        $linkedArtefactIds = array_column($artefactSummary, 'artefactid');
        foreach ($artefactService->getArtefactsForProject($activitydata['projectid']) as $projectArtefact) {
            if (in_array($projectArtefact['artefactid'], $linkedArtefactIds)) continue;
            $remaining = round(1 - array_sum(array_column($projectArtefact['activities'], 'ratio')), 2);
            if ($remaining <= 0) continue;
            $projectArtefact['remainingratio'] = $remaining;
            $linkableArtefacts[] = $projectArtefact;
        }
    }

    // Associating milestones is limited to the activity manager, project Leads and the project Manager
    $projectMilestones = $canManageActivity ? $milestoneService->getMilestonesForProject($activitydata['projectid']) : [];

    $pageTitle = $activitydata['activity_name'];
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
                        <li class="breadcrumb-item"><?php echo htmlspecialchars($activitydata['project_name']); ?></li>
                        <li class="breadcrumb-item active" aria-current="page"><?php echo htmlspecialchars($activitydata['activity_name']); ?></li>
                    </ol>
                </nav>
                <div class="section-label">Activity</div>
                <h1 class="h3"><?php echo htmlspecialchars($activitydata['activity_name']); ?></h1>
                <p class="page-subtitle">
                    <?php echo htmlspecialchars($activitydata['project_number'] . ' ' . $activitydata['project_name']); ?>
                    &middot; <?php echo htmlspecialchars($activitydata['category'] . ' / ' . $activitydata['subcategory']); ?>
                </p>
            </div>
        </div>

        <?php if ($message): ?>
            <div class="alert alert-<?php echo $messageType; ?> alert-dismissible fade show" role="alert">
                <?php echo htmlspecialchars($message); ?>
                <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Close"></button>
            </div>
        <?php endif; ?>

        <div class="row g-4 mb-4">
            <div class="col-lg-7">
                <!-- Overview -->
                <div class="card h-100">
                    <div class="card-header">
                        <h2 class="card-title">Overview</h2>
                        <?php if ($canManageActivity): ?>
                            <div class="card-actions">
                                <button type="button" class="btn btn-sm btn-outline-secondary" data-bs-toggle="modal" data-bs-target="#editActivityModal">
                                    <i class="bi bi-pencil"></i> Edit Activity
                                </button>
                                <?php if ($canDelegateActivity): ?>
                                    <button type="button" class="btn btn-sm btn-outline-secondary" data-bs-toggle="modal" data-bs-target="#delegateActivityModal">
                                        <i class="bi bi-person-up"></i> Delegate Activity
                                    </button>
                                <?php endif; ?>
                            </div>
                        <?php endif; ?>
                    </div>
                    <div class="card-body">
                        <?php if (!empty($activitydata['description'])): ?>
                            <p><?php echo htmlspecialchars($activitydata['description']); ?></p>
                        <?php endif; ?>
                        <dl class="row detail-list">
                            <dt class="col-sm-4">Category</dt>
                            <dd class="col-sm-8"><?php echo htmlspecialchars($activitydata['category']); ?></dd>
                            <dt class="col-sm-4">Sub-category</dt>
                            <dd class="col-sm-8"><?php echo htmlspecialchars($activitydata['subcategory']); ?></dd>
                            <dt class="col-sm-4">Activity manager</dt>
                            <dd class="col-sm-8"><?php echo htmlspecialchars($activitydata['activity_manager']); ?></dd>
                        </dl>

                        <!-- Gantt Chart -->
                        <div class="section-label border-top pt-3 mb-2">Schedule</div>
                        <div class="mb-4">
                        <?php
                        // Parse dates and handle null/empty values
                        $plannedStart = !empty($activitydata['planned_start']) ? new DateTime($activitydata['planned_start']) : null;
                        $plannedFinish = !empty($activitydata['planned_finish']) ? new DateTime($activitydata['planned_finish']) : null;
                        $forecastStart = !empty($activitydata['forecast_start']) ? new DateTime($activitydata['forecast_start']) : null;
                        $forecastFinish = !empty($activitydata['forecast_finish']) ? new DateTime($activitydata['forecast_finish']) : null;
                        $actualStart = !empty($activitydata['actual_start']) ? new DateTime($activitydata['actual_start']) : null;
                        $actualFinish = !empty($activitydata['actual_finish']) ? new DateTime($activitydata['actual_finish']) : null;
                        $ganttChart = new GanttChart($plannedStart, $plannedFinish, $forecastStart, $forecastFinish, $actualStart, $actualFinish);
                        echo $ganttChart->draw();
                        ?>
                        </div>

                        <!-- Hours Chart -->
                        <div class="section-label border-top pt-3 mb-2">Hours</div>
                        <div>
                        <?php
                        $plannedHours = (float)($activitydata['planned_hours'] ?? 0);
                        $forecastHours = (float)($activitydata['forecast_hours'] ?? 0);
                        $consumedHours = (float)($activitydata['consumedhours'] ?? 0);
                        $hoursChart = new HoursChart($plannedHours,$forecastHours,$consumedHours);
                        echo $hoursChart->draw();
                        ?>
                        </div>
                    </div>
                </div>
            </div>

            <div class="col-lg-5">
                <!-- Task Tree -->
                <div class="card h-100">
                    <div class="card-header">
                        <h2 class="card-title">Tasks</h2>
                        <?php if ($canManageActivity): ?>
                            <div class="card-actions">
                                <button type="button" class="btn btn-sm btn-primary" data-bs-toggle="modal" data-bs-target="#addTaskModal">
                                    <i class="bi bi-plus-lg"></i> Add Task
                                </button>
                                <button type="button" class="btn btn-sm btn-outline-secondary" data-bs-toggle="modal" data-bs-target="#assignTaskModal">
                                    <i class="bi bi-person-plus"></i> Assign Task
                                </button>
                                <button type="button" class="btn btn-sm btn-outline-secondary" data-bs-toggle="modal" data-bs-target="#reassignOpenTaskModal">
                                    <i class="bi bi-arrow-left-right"></i> Reassign Task
                                </button>
                            </div>
                        <?php endif; ?>
                    </div>
                    <div class="card-body">
                        <?php
                        $taskTree = new TaskTree($taskHeirarchy,$activitydata['activity_name']);
                        echo $taskTree->draw();
                        ?>
                    </div>
                </div>
            </div>
        </div>

        <!-- My Tasks -->
        <div class="card mb-4">
            <div class="card-header">
                <h2 class="card-title">My Tasks</h2>
                <?php if (!empty($userTasks)): ?>
                    <div class="card-actions">
                        <button type="button" class="btn btn-sm btn-outline-secondary task-selection-action" data-bs-toggle="modal" data-bs-target="#closeTaskModal" disabled>
                            <?php if ($isActivityManager): ?>
                                <i class="bi bi-check2-circle"></i> Close Task
                            <?php else: ?>
                                <i class="bi bi-send-check"></i> Complete Task
                            <?php endif; ?>
                        </button>
                    </div>
                <?php endif; ?>
            </div>
            <?php if (!empty($userTasks)): ?>
                <div class="table-responsive">
                <table class="table table-hover align-middle mb-0" id="myTasksTable">
                    <thead>
                        <tr>
                            <th class="row-select" style="width: 1%;">
                                <input class="form-check-input" type="checkbox" id="selectAllTasks" aria-label="Select all tasks">
                            </th>
                            <th>Task</th>
                            <th>Assigned</th>
                            <th class="text-end">Booked hours</th>
                        </tr>
                    </thead>
                    <tbody>
                        <?php foreach ($userTasks as $task): ?>
                            <tr class="clickable-row" data-href="tasks.php?taskid=<?php echo (int)$task['taskid']; ?>">
                                <td class="row-select">
                                    <input class="form-check-input task-select" type="checkbox"
                                           value="<?php echo (int)$task['assignmentid']; ?>"
                                           data-task-name="<?php echo htmlspecialchars($task['task_hierarchy'], ENT_QUOTES); ?>"
                                           aria-label="Select <?php echo htmlspecialchars($task['task_hierarchy'], ENT_QUOTES); ?>">
                                </td>
                                <td class="fw-medium"><?php echo htmlspecialchars($task['task_hierarchy']); ?></td>
                                <td><?php echo htmlspecialchars($task['assigned_date']); ?></td>
                                <td class="text-end"><?php echo htmlspecialchars($task['booked_hours']); ?></td>
                            </tr>
                        <?php endforeach; ?>
                    </tbody>
                </table>
                </div>
            <?php else: ?>
                <div class="empty-state">
                    <i class="bi bi-check2-all"></i>
                    No open tasks assigned to you in this activity.
                </div>
            <?php endif; ?>
        </div>

        <!-- Artefacts -->
        <div class="card mb-4">
            <div class="card-header">
                <h2 class="card-title">Artefacts</h2>
                <?php if ($canLinkArtefacts): ?>
                    <div class="card-actions">
                        <?php if ($canLinkArtefacts): ?>
                            <button type="button" class="btn btn-sm btn-outline-secondary" data-bs-toggle="modal" data-bs-target="#linkArtefactModal">
                                <i class="bi bi-link-45deg"></i> Link Artefact
                            </button>
                        <?php endif; ?>
                        <?php if ($canManageActivity): ?>
                            <button type="button" class="btn btn-sm btn-outline-secondary artefact-selection-action" data-bs-toggle="modal" data-bs-target="#associateMilestoneModal" disabled>
                                <i class="bi bi-flag"></i> Set Milestones
                            </button>
                        <?php endif; ?>
                        <?php if ($canLinkArtefacts): ?>
                            <button type="button" class="btn btn-sm btn-outline-danger artefact-selection-action" data-bs-toggle="modal" data-bs-target="#unlinkArtefactModal" disabled>
                                <i class="bi bi-x-lg"></i> Remove Link
                            </button>
                        <?php endif; ?>
                    </div>
                <?php endif; ?>
            </div>
            <?php if (!empty($artefactSummary)): ?>
                <div class="table-responsive">
                    <table class="table table-hover align-middle artefact-table mb-0">
                        <thead>
                            <tr>
                                <?php if ($canLinkArtefacts): ?>
                                    <th class="row-select" style="width: 1%;">
                                        <input class="form-check-input" type="checkbox" id="selectAllArtefacts" aria-label="Select all artefacts">
                                    </th>
                                <?php endif; ?>
                                <th>Artefact</th>
                                <th>Type</th>
                                <th>Owner</th>
                                <th>Milestones Completed</th>
                                <th>Progress</th>
                            </tr>
                        </thead>
                        <tbody>
                            <?php foreach ($artefactSummary as $artefact): ?>
                                <?php $artefactUrl = 'artefact.php?artefactlinkid=' . urlencode($artefact['artefactlinkid']); ?>
                                <tr class="clickable-row" data-href="<?php echo $artefactUrl; ?>">
                                    <?php if ($canLinkArtefacts): ?>
                                        <td class="row-select">
                                            <input class="form-check-input artefact-select" type="checkbox"
                                                   value="<?php echo (int)$artefact['artefactlinkid']; ?>"
                                                   data-artefact-title="<?php echo htmlspecialchars($artefact['title'], ENT_QUOTES); ?>"
                                                   aria-label="Select <?php echo htmlspecialchars($artefact['title'], ENT_QUOTES); ?>">
                                        </td>
                                    <?php endif; ?>
                                    <td>
                                        <a href="<?php echo $artefactUrl; ?>" class="text-reset text-decoration-none fw-medium">
                                            <?php echo htmlspecialchars($artefact['title']); ?>
                                        </a>
                                    </td>
                                    <td class="text-body-secondary"><?php echo htmlspecialchars($artefact['artefact']); ?></td>
                                    <td><?php echo $artefact['artefactowner'] !== null ? htmlspecialchars($artefact['artefactowner']) : '&mdash;'; ?></td>
                                    <td class="small text-body-secondary">
                                        <div class="d-flex align-items-center gap-2">
                                            <button type="button" class="btn btn-sm btn-outline-secondary btn-icon flex-shrink-0" data-bs-toggle="modal"
                                                    data-bs-target="#setMilestonesModal-<?php echo (int)$artefact['artefactlinkid']; ?>"
                                                    title="Set milestones" aria-label="Set milestones for <?php echo htmlspecialchars($artefact['title'], ENT_QUOTES); ?>">
                                                <i class="bi bi-flag"></i>
                                            </button>
                                            <span><?php echo $artefact['completedmilestones'] !== null ? htmlspecialchars($artefact['completedmilestones']) : '&mdash;'; ?></span>
                                        </div>
                                    </td>
                                    <td>
                                        <div class="d-flex align-items-center gap-2">
                                            <div class="progress flex-grow-1" role="progressbar"
                                                 aria-valuenow="<?php echo (float)$artefact['progress']; ?>" aria-valuemin="0" aria-valuemax="100">
                                                <div class="progress-bar <?php echo $artefact['progress'] == 100 ? 'is-complete' : ''; ?>"
                                                     style="width: <?php echo (float)$artefact['progress']; ?>%"></div>
                                            </div>
                                            <span class="small text-body-secondary" style="min-width: 2.5rem;"><?php echo number_format($artefact['progress'], 0); ?>%</span>
                                        </div>
                                    </td>
                                </tr>
                            <?php endforeach; ?>
                        </tbody>
                    </table>
                </div>
            <?php else: ?>
                <div class="empty-state">
                    <i class="bi bi-folder2-open"></i>
                    No artefacts associated with this activity.
                </div>
            <?php endif; ?>
        </div>

        <!-- Activity Notes -->
        <div class="card">
            <div class="card-header">
                <h2 class="card-title">Notes</h2>
                <div class="card-actions">
                    <button type="button" class="btn btn-sm btn-primary" data-bs-toggle="modal" data-bs-target="#addNoteModal">
                        <i class="bi bi-plus-lg"></i> Add Note
                    </button>
                </div>
            </div>
            <?php if (!empty($notelist)): ?>
                <ul class="list-group list-group-flush">
                    <?php foreach ($notelist as $note): ?>
                        <li class="list-group-item note-item">
                            <div class="note-text mb-1"><?php echo htmlspecialchars($note['note']); ?></div>
                            <div class="note-meta d-flex justify-content-between align-items-center">
                                <span>
                                    <?php echo htmlspecialchars($note['note_by']); ?> &middot; <?php echo htmlspecialchars($note['note_date']); ?>
                                </span>
                                <?php if ($note['user_id'] == $eid): ?>
                                    <div class="d-flex gap-1">
                                        <button type="button" class="btn btn-sm btn-link text-body-secondary btn-icon"
                                                title="Edit this note" aria-label="Edit this note"
                                                data-bs-toggle="modal" data-bs-target="#editNoteModal"
                                                data-note-id="<?php echo (int)$note['note_id']; ?>"
                                                data-note="<?php echo htmlspecialchars($note['note'], ENT_QUOTES); ?>"
                                                data-note-date="<?php echo htmlspecialchars($note['note_date'], ENT_QUOTES); ?>">
                                            <i class="bi bi-pencil"></i>
                                        </button>
                                        <form method="POST" class="m-0"
                                              onsubmit="return confirm('Are you sure you want to delete this note?');">
                                            <input type="hidden" name="noteid" value="<?php echo $note['note_id']; ?>">
                                            <input type="hidden" name="activityid" value="<?php echo $activityid; ?>">
                                            <?php echo Csrf::field(); ?>
                                            <button type="submit" name="delete_note" class="btn btn-sm btn-link text-danger btn-icon"
                                                    title="Delete this note" aria-label="Delete this note">
                                                <i class="bi bi-trash"></i>
                                            </button>
                                        </form>
                                    </div>
                                <?php endif; ?>
                            </div>
                        </li>
                    <?php endforeach; ?>
                </ul>
            <?php else: ?>
                <div class="empty-state">
                    <i class="bi bi-journal-text"></i>
                    No notes available for this activity.
                </div>
            <?php endif; ?>
        </div>
    </main>

    <!-- Add Note Modal -->
    <div class="modal fade" id="addNoteModal" tabindex="-1" aria-labelledby="addNoteModalLabel" aria-hidden="true">
        <div class="modal-dialog">
            <div class="modal-content">
                <form method="POST">
                    <input type="hidden" name="activityid" value="<?php echo $activityid; ?>">
                    <?php echo Csrf::field(); ?>
                    <div class="modal-header">
                        <h5 class="modal-title" id="addNoteModalLabel">Add Activity Note</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <div class="mb-3">
                            <label for="notedate" class="form-label">Date</label>
                            <input type="date" class="form-control" id="notedate"
                                   value="<?php echo date('Y-m-d'); ?>" readonly>
                        </div>
                        <div>
                            <label for="note" class="form-label">Note</label>
                            <textarea class="form-control" id="note" name="note" rows="4"
                                      placeholder="Enter your note here..." required></textarea>
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <button type="submit" name="add_note" class="btn btn-primary">Add Note</button>
                    </div>
                </form>
            </div>
        </div>
    </div>

    <!-- Edit Note Modal -->
    <div class="modal fade" id="editNoteModal" tabindex="-1" aria-labelledby="editNoteModalLabel" aria-hidden="true">
        <div class="modal-dialog">
            <div class="modal-content">
                <form method="POST">
                    <input type="hidden" name="activityid" value="<?php echo $activityid; ?>">
                    <?php echo Csrf::field(); ?>
                    <input type="hidden" name="noteid" id="editNoteId">
                    <div class="modal-header">
                        <h5 class="modal-title" id="editNoteModalLabel">Edit Activity Note</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <div class="mb-3">
                            <label for="editNoteDate" class="form-label">Date</label>
                            <input type="date" class="form-control" id="editNoteDate" readonly>
                        </div>
                        <div>
                            <label for="editNote" class="form-label">Note</label>
                            <textarea class="form-control" id="editNote" name="note" rows="4"
                                      placeholder="Enter your note here..." required></textarea>
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <button type="submit" name="edit_note" class="btn btn-primary">Update Note</button>
                    </div>
                </form>
            </div>
        </div>
    </div>

    <!-- Add Task Modal -->
    <div class="modal fade" id="addTaskModal" tabindex="-1" aria-labelledby="addTaskModalLabel" aria-hidden="true">
        <div class="modal-dialog">
            <div class="modal-content">
                <form method="POST">
                    <input type="hidden" name="activityid" value="<?php echo $activityid; ?>">
                    <?php echo Csrf::field(); ?>
                    <div class="modal-header">
                        <h5 class="modal-title" id="addTaskModalLabel">Add Task</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <div class="mb-3">
                            <label for="taskname" class="form-label">Task name</label>
                            <input type="text" class="form-control" id="taskname" name="taskname"
                                   placeholder="Enter task name..." required>
                        </div>
                        <div>
                            <label for="parenttaskid" class="form-label">Parent task</label>
                            <select class="form-select" id="parenttaskid" name="parenttaskid">
                                <option value="">No Parent (Root Level Task)</option>
                                <?php foreach ($tasksForParentSelection as $task): ?>
                                    <option value="<?php echo $task['taskid']; ?>">
                                        <?php echo htmlspecialchars($task['taskname']); ?>
                                    </option>
                                <?php endforeach; ?>
                            </select>
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <button type="submit" name="add_task" class="btn btn-primary">Add Task</button>
                    </div>
                </form>
            </div>
        </div>
    </div>

    <!-- Assign Task Modal -->
    <div class="modal fade" id="assignTaskModal" tabindex="-1" aria-labelledby="assignTaskModalLabel" aria-hidden="true">
        <div class="modal-dialog">
            <div class="modal-content">
                <form method="POST">
                    <input type="hidden" name="activityid" value="<?php echo $activityid; ?>">
                    <?php echo Csrf::field(); ?>
                    <div class="modal-header">
                        <h5 class="modal-title" id="assignTaskModalLabel">Assign Task</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <?php if (!empty($unassignedTasks)): ?>
                            <div class="mb-3">
                                <label for="assignTaskSelect" class="form-label">Unassigned task</label>
                                <select class="form-select" id="assignTaskSelect" name="assign_taskid" required>
                                    <option value="">Please select a task...</option>
                                    <?php foreach ($unassignedTasks as $task): ?>
                                        <option value="<?php echo $task['taskid']; ?>">
                                            <?php echo htmlspecialchars($task['taskname']); ?>
                                        </option>
                                    <?php endforeach; ?>
                                </select>
                            </div>
                            <div class="mb-3">
                                <label for="assignUserSelect" class="form-label">Assign to</label>
                                <select class="form-select" id="assignUserSelect" name="assign_userid" required>
                                    <option value="">Please select a user...</option>
                                    <?php foreach ($activityTeamMembers as $employee): ?>
                                        <option value="<?php echo $employee['employeeid']; ?>">
                                            <?php echo htmlspecialchars($employee['fullname']); ?>
                                        </option>
                                    <?php endforeach; ?>
                                </select>
                            </div>
                            <div>
                                <label for="assigneddate" class="form-label">Assignment date</label>
                                <input type="date" class="form-control" id="assigneddate"
                                       value="<?php echo date('Y-m-d'); ?>" readonly>
                            </div>
                        <?php else: ?>
                            <div class="alert alert-info mb-0">
                                No unassigned tasks available. All tasks in this activity are currently assigned.
                            </div>
                        <?php endif; ?>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <?php if (!empty($unassignedTasks)): ?>
                            <button type="submit" name="assign_task" class="btn btn-primary">Assign Task</button>
                        <?php endif; ?>
                    </div>
                </form>
            </div>
        </div>
    </div>

    <!-- Edit Activity Modal -->
    <div class="modal fade" id="editActivityModal" tabindex="-1" aria-labelledby="editActivityModalLabel" aria-hidden="true">
        <div class="modal-dialog modal-lg">
            <div class="modal-content">
                <form method="POST">
                    <input type="hidden" name="activityid" value="<?php echo $activityid; ?>">
                    <?php echo Csrf::field(); ?>
                    <div class="modal-header">
                        <h5 class="modal-title" id="editActivityModalLabel">Edit Activity Details</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <div class="mb-3">
                            <label for="edit_activityname" class="form-label">Activity name</label>
                            <input type="text" class="form-control" id="edit_activityname" name="edit_activityname"
                                   value="<?php echo htmlspecialchars($activitydata['activity_name']); ?>" required>
                        </div>
                        <div class="mb-4">
                            <label for="edit_activitydescription" class="form-label">Description</label>
                            <textarea class="form-control" id="edit_activitydescription" name="edit_activitydescription" rows="3"><?php echo htmlspecialchars($activitydata['description'] ?? ''); ?></textarea>
                        </div>
                        <div class="row g-3">
                            <div class="col-sm-4">
                                <div class="section-label mb-2">Planned</div>
                                <div class="mb-2">
                                    <label for="edit_plannedstart" class="form-label small">Start</label>
                                    <input type="date" class="form-control" id="edit_plannedstart" name="edit_plannedstart"
                                           value="<?php echo $activitydata['planned_start']; ?>">
                                </div>
                                <div class="mb-2">
                                    <label for="edit_plannedfinish" class="form-label small">Finish</label>
                                    <input type="date" class="form-control" id="edit_plannedfinish" name="edit_plannedfinish"
                                           value="<?php echo $activitydata['planned_finish']; ?>">
                                </div>
                                <div>
                                    <label for="edit_plannedhours" class="form-label small">Hours</label>
                                    <input type="number" step="0.01" class="form-control" id="edit_plannedhours" name="edit_plannedhours"
                                           value="<?php echo $activitydata['planned_hours']; ?>">
                                </div>
                            </div>
                            <div class="col-sm-4">
                                <div class="section-label mb-2">Forecast</div>
                                <div class="mb-2">
                                    <label for="edit_forecaststart" class="form-label small">Start</label>
                                    <input type="date" class="form-control" id="edit_forecaststart" name="edit_forecaststart"
                                           value="<?php echo $activitydata['forecast_start']; ?>">
                                </div>
                                <div class="mb-2">
                                    <label for="edit_forecastfinish" class="form-label small">Finish</label>
                                    <input type="date" class="form-control" id="edit_forecastfinish" name="edit_forecastfinish"
                                           value="<?php echo $activitydata['forecast_finish']; ?>">
                                </div>
                                <div>
                                    <label for="edit_forecasthours" class="form-label small">Hours</label>
                                    <input type="number" step="0.01" class="form-control" id="edit_forecasthours" name="edit_forecasthours"
                                           value="<?php echo $activitydata['forecast_hours']; ?>">
                                </div>
                            </div>
                            <div class="col-sm-4">
                                <div class="section-label mb-2">Actual</div>
                                <div class="mb-2">
                                    <label for="edit_actualstart" class="form-label small">Start</label>
                                    <input type="date" class="form-control" id="edit_actualstart"
                                           value="<?php echo $activitydata['actual_start']; ?>" disabled>
                                </div>
                                <div class="mb-2">
                                    <label for="edit_actualfinish" class="form-label small">Finish</label>
                                    <input type="date" class="form-control" id="edit_actualfinish"
                                           value="<?php echo $activitydata['actual_finish']; ?>" disabled>
                                </div>
                                <div class="form-text">Calculated from task assignments.</div>
                            </div>
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <button type="submit" name="edit_activity" class="btn btn-primary">Update Activity</button>
                    </div>
                </form>
            </div>
        </div>
    </div>

    <?php if ($canManageActivity): ?>
        <!-- Reassign Open Task Modal (Tasks card) -->
        <div class="modal fade" id="reassignOpenTaskModal" tabindex="-1" aria-labelledby="reassignOpenTaskModalLabel" aria-hidden="true">
            <div class="modal-dialog">
                <div class="modal-content">
                    <form method="POST">
                        <input type="hidden" name="activityid" value="<?php echo (int)$activityid; ?>">
                        <?php echo Csrf::field(); ?>
                        <div class="modal-header">
                            <h5 class="modal-title" id="reassignOpenTaskModalLabel">Reassign Task</h5>
                            <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                        </div>
                        <div class="modal-body">
                            <?php if (!empty($openTaskAssignments)): ?>
                                <div class="mb-3">
                                    <label for="reassignOpenAssignmentSelect" class="form-label">Open task</label>
                                    <select class="form-select" id="reassignOpenAssignmentSelect" name="reassign_open_assignmentid" required>
                                        <option value="">Please select a task...</option>
                                        <?php foreach ($openTaskAssignments as $assignment): ?>
                                            <option value="<?php echo (int)$assignment['assignmentid']; ?>">
                                                <?php echo htmlspecialchars($assignment['task_hierarchy'] . ' (' . $assignment['assignee'] . ')'); ?>
                                            </option>
                                        <?php endforeach; ?>
                                    </select>
                                </div>
                                <div class="mb-3">
                                    <label for="reassignOpenUserSelect" class="form-label">Reassign to</label>
                                    <select class="form-select" id="reassignOpenUserSelect" name="reassign_open_userid" required>
                                        <option value="">Please select a user...</option>
                                        <?php foreach ($activityTeamMembers as $employee): ?>
                                            <option value="<?php echo (int)$employee['employeeid']; ?>">
                                                <?php echo htmlspecialchars($employee['fullname']); ?>
                                            </option>
                                        <?php endforeach; ?>
                                    </select>
                                </div>
                                <div>
                                    <label for="reassignOpenDate" class="form-label">Reassignment date</label>
                                    <input type="date" class="form-control" id="reassignOpenDate"
                                           value="<?php echo date('Y-m-d'); ?>" readonly>
                                </div>
                            <?php else: ?>
                                <div class="alert alert-info mb-0">
                                    No open tasks available. There are no open task assignments in this activity.
                                </div>
                            <?php endif; ?>
                        </div>
                        <div class="modal-footer">
                            <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                            <?php if (!empty($openTaskAssignments)): ?>
                                <button type="submit" name="reassign_open_task" class="btn btn-primary">Reassign</button>
                            <?php endif; ?>
                        </div>
                    </form>
                </div>
            </div>
        </div>
    <?php endif; ?>

    <!-- Close Task Modal -->
    <div class="modal fade" id="closeTaskModal" tabindex="-1" aria-labelledby="closeTaskModalLabel" aria-hidden="true">
        <div class="modal-dialog">
            <div class="modal-content">
                <form method="POST">
                    <input type="hidden" name="activityid" value="<?php echo $activityid; ?>">
                    <?php echo Csrf::field(); ?>
                    <div class="selected-task-inputs"></div>
                    <div class="modal-header">
                        <h5 class="modal-title" id="closeTaskModalLabel">
                            <?php echo $isActivityManager ? 'Close Tasks' : 'Complete Tasks'; ?>
                        </h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <p class="mb-3"><span class="text-body-secondary">Tasks:</span> <strong class="selected-task-names"></strong></p>
                        <div class="alert <?php echo $isActivityManager ? 'alert-success' : 'alert-warning'; ?>">
                            <?php if ($isActivityManager): ?>
                                This will mark the selected tasks as completed and close them permanently.
                            <?php else: ?>
                                This will complete your assignments and reassign the selected tasks to the activity manager.
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

    <?php if ($canDelegateActivity): ?>
    <!-- Delegate Activity Modal -->
    <div class="modal fade" id="delegateActivityModal" tabindex="-1" aria-labelledby="delegateActivityModalLabel" aria-hidden="true">
        <div class="modal-dialog">
            <div class="modal-content">
                <form method="POST">
                    <input type="hidden" name="activityid" value="<?php echo $activityid; ?>">
                    <?php echo Csrf::field(); ?>
                    <div class="modal-header">
                        <h5 class="modal-title" id="delegateActivityModalLabel">Delegate Activity</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <label for="newmanagerid" class="form-label">Activity manager</label>
                        <select class="form-select" id="newmanagerid" name="newmanagerid" required>
                            <?php foreach ($projectTeamMembers as $member): ?>
                                <option value="<?php echo (int)$member['employeeid']; ?>"<?php echo $member['employeeid'] == $activityManagerId ? ' selected' : ''; ?>>
                                    <?php echo htmlspecialchars($member['fullname']); ?>
                                </option>
                            <?php endforeach; ?>
                        </select>
                        <div class="form-text">Only members of this project team can manage the activity.</div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <button type="submit" name="delegate_activity" class="btn btn-primary">Delegate</button>
                    </div>
                </form>
            </div>
        </div>
    </div>
    <?php endif; ?>

    <?php if ($canLinkArtefacts): ?>
    <!-- Link Artefact Modal -->
    <div class="modal fade" id="linkArtefactModal" tabindex="-1" aria-labelledby="linkArtefactModalLabel" aria-hidden="true">
        <div class="modal-dialog">
            <div class="modal-content">
                <form method="POST">
                    <input type="hidden" name="activityid" value="<?php echo $activityid; ?>">
                    <?php echo Csrf::field(); ?>
                    <div class="modal-header">
                        <h5 class="modal-title" id="linkArtefactModalLabel">Link Existing Artefact</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <?php if (!empty($linkableArtefacts)): ?>
                            <div class="mb-3">
                                <label for="linkArtefactSelect" class="form-label">Artefact</label>
                                <select class="form-select" id="linkArtefactSelect" name="link_artefactid" required>
                                    <option value="">Please select an artefact...</option>
                                    <?php foreach ($linkableArtefacts as $linkable): ?>
                                        <option value="<?php echo (int)$linkable['artefactid']; ?>" data-remaining-ratio="<?php echo $linkable['remainingratio']; ?>">
                                            <?php echo htmlspecialchars($linkable['artefact'] . ' / ' . $linkable['title']); ?>
                                        </option>
                                    <?php endforeach; ?>
                                </select>
                            </div>
                            <div>
                                <label for="linkRatio" class="form-label">Ratio</label>
                                <input type="number" class="form-control" id="linkRatio" name="link_ratio"
                                       min="0.01" max="1" step="0.01" required>
                                <div class="form-text" id="linkRatioHelp">Share of the artefact's work done in this activity.</div>
                            </div>
                        <?php else: ?>
                            <div class="alert alert-info mb-0">
                                No artefacts available to link. Every artefact in this project is either already linked to this activity or fully allocated to other activities.
                            </div>
                        <?php endif; ?>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <?php if (!empty($linkableArtefacts)): ?>
                            <button type="submit" name="link_artefact" class="btn btn-primary">Link Artefact</button>
                        <?php endif; ?>
                    </div>
                </form>
            </div>
        </div>
    </div>

    <!-- Remove Linked Artefacts Modal -->
    <div class="modal fade" id="unlinkArtefactModal" tabindex="-1" aria-labelledby="unlinkArtefactModalLabel" aria-hidden="true">
        <div class="modal-dialog">
            <div class="modal-content">
                <form method="POST">
                    <input type="hidden" name="activityid" value="<?php echo $activityid; ?>">
                    <?php echo Csrf::field(); ?>
                    <div class="selected-artefact-inputs"></div>
                    <div class="modal-header">
                        <h5 class="modal-title" id="unlinkArtefactModalLabel">Remove Linked Artefacts</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <p class="mb-3"><span class="text-body-secondary">Artefacts:</span> <strong class="selected-artefact-titles"></strong></p>
                        <div class="alert alert-warning mb-0">
                            <i class="bi bi-exclamation-triangle me-1"></i>
                            The artefacts will no longer be linked to this activity, and milestone progress recorded against them for this activity will be deleted. The artefacts themselves are not deleted.
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <button type="submit" name="unlink_artefact" class="btn btn-danger">Remove</button>
                    </div>
                </form>
            </div>
        </div>
    </div>
    <?php endif; ?>

    <?php if ($canManageActivity): ?>
    <!-- Associate Milestone Modal -->
    <div class="modal fade" id="associateMilestoneModal" tabindex="-1" aria-labelledby="associateMilestoneModalLabel" aria-hidden="true">
        <div class="modal-dialog">
            <div class="modal-content">
                <form method="POST">
                    <input type="hidden" name="activityid" value="<?php echo $activityid; ?>">
                    <?php echo Csrf::field(); ?>
                    <div class="selected-artefact-inputs"></div>
                    <div class="modal-header">
                        <h5 class="modal-title" id="associateMilestoneModalLabel">Associate Milestone</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <p class="mb-3"><span class="text-body-secondary">Artefacts:</span> <strong class="selected-artefact-titles"></strong></p>
                        <?php if (!empty($projectMilestones)): ?>
                            <div class="mb-3">
                                <label for="associateMilestoneSelect" class="form-label">Milestone</label>
                                <select class="form-select" id="associateMilestoneSelect" name="milestoneid" required>
                                    <option value="">Please select a milestone...</option>
                                    <?php foreach ($projectMilestones as $milestone): ?>
                                        <option value="<?php echo (int)$milestone['milestoneid']; ?>" <?php echo empty($milestone['steps']) ? 'disabled' : ''; ?>>
                                            <?php echo htmlspecialchars($milestone['milestonename']); ?><?php echo empty($milestone['steps']) ? ' (no steps)' : ''; ?>
                                        </option>
                                    <?php endforeach; ?>
                                </select>
                            </div>
                            <?php foreach ($projectMilestones as $milestone): ?>
                                <?php if (empty($milestone['steps'])) continue; ?>
                                <fieldset class="milestone-step-dates d-none" data-milestone-id="<?php echo (int)$milestone['milestoneid']; ?>" disabled>
                                    <legend class="form-label fs-6">Planned dates <span class="text-body-secondary small">(optional)</span></legend>
                                    <?php foreach ($milestone['steps'] as $step): ?>
                                        <?php $dateId = 'plannedDate-' . (int)$step['milestonestepid']; ?>
                                        <div class="row g-2 align-items-center mb-2">
                                            <label for="<?php echo $dateId; ?>" class="col-sm-6 col-form-label col-form-label-sm">
                                                <?php echo htmlspecialchars($step['milestonestepname']); ?>
                                                <span class="text-body-secondary">&middot; <?php echo number_format($step['progressratio'] * 100, 0); ?>%</span>
                                            </label>
                                            <div class="col-sm-6">
                                                <input type="date" class="form-control form-control-sm" id="<?php echo $dateId; ?>"
                                                       name="planneddates[<?php echo (int)$step['milestonestepid']; ?>]">
                                            </div>
                                        </div>
                                    <?php endforeach; ?>
                                </fieldset>
                            <?php endforeach; ?>
                            <div class="form-text">Artefacts that already have a milestone associated are skipped.</div>
                        <?php else: ?>
                            <div class="alert alert-info mb-0">
                                This project has no milestones defined.
                            </div>
                        <?php endif; ?>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <?php if (!empty($projectMilestones)): ?>
                            <button type="submit" name="associate_milestone" class="btn btn-primary">Associate</button>
                        <?php endif; ?>
                    </div>
                </form>
            </div>
        </div>
    </div>
    <?php endif; ?>

    <!-- Set Milestones Modals (one per linked artefact, since each has its own steps) -->
    <?php foreach ($artefactSummary as $artefact): ?>
        <?php $modalId = 'setMilestonesModal-' . (int)$artefact['artefactlinkid']; ?>
        <div class="modal fade" id="<?php echo $modalId; ?>" tabindex="-1" aria-labelledby="<?php echo $modalId; ?>-label" aria-hidden="true">
            <div class="modal-dialog">
                <div class="modal-content">
                    <form method="POST">
                        <input type="hidden" name="activityid" value="<?php echo $activityid; ?>">
                        <?php echo Csrf::field(); ?>
                        <input type="hidden" name="milestones_artefactlinkid" value="<?php echo (int)$artefact['artefactlinkid']; ?>">
                        <div class="modal-header">
                            <h5 class="modal-title" id="<?php echo $modalId; ?>-label">Set Milestones</h5>
                            <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                        </div>
                        <div class="modal-body">
                            <p class="mb-3"><span class="text-body-secondary">Artefact:</span> <strong><?php echo htmlspecialchars($artefact['title']); ?></strong></p>
                            <?php if (empty($artefact['milestonesteps'])): ?>
                                <div class="empty-state">
                                    <i class="bi bi-flag"></i>
                                    No milestone steps are linked to this artefact.
                                </div>
                            <?php else: ?>
                                <div class="mb-3">
                                    <?php foreach ($artefact['milestonesteps'] as $step): ?>
                                        <?php $checkId = $modalId . '-step-' . (int)$step['milestonelinkid']; ?>
                                        <div class="form-check">
                                            <input class="form-check-input" type="checkbox" name="completedlinkids[]" id="<?php echo $checkId; ?>"
                                                   value="<?php echo (int)$step['milestonelinkid']; ?>" <?php echo $step['completiondate'] !== null ? 'checked' : ''; ?>
                                                   <?php echo $step['completiondate'] !== null && !$isProjectLeadOrManager ? 'disabled title="Only a project lead or the project manager can untick a completed step"' : ''; ?>>
                                            <label class="form-check-label" for="<?php echo $checkId; ?>">
                                                <?php echo htmlspecialchars($step['milestonestepname']); ?>
                                                <?php if ($step['completiondate'] !== null): ?>
                                                    <span class="small text-body-secondary">&middot; completed <?php echo htmlspecialchars($step['completiondate']); ?> by <?php echo htmlspecialchars($step['completed_by']); ?></span>
                                                <?php endif; ?>
                                            </label>
                                        </div>
                                    <?php endforeach; ?>
                                </div>
                                <label for="<?php echo $modalId; ?>-date" class="form-label">Completion date for newly ticked steps</label>
                                <input type="date" class="form-control" id="<?php echo $modalId; ?>-date" name="milestonecompletiondate"
                                       value="<?php echo date('Y-m-d'); ?>" required>
                            <?php endif; ?>
                        </div>
                        <div class="modal-footer">
                            <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                            <?php if (!empty($artefact['milestonesteps'])): ?>
                                <button type="submit" name="set_milestones" class="btn btn-primary">Save Milestones</button>
                            <?php endif; ?>
                        </div>
                    </form>
                </div>
            </div>
        </div>
    <?php endforeach; ?>

    <?php include 'includes/footer.php'; ?>
    <script>
    function onModalShow(id, handler) {
        document.getElementById(id).addEventListener('show.bs.modal', function (event) {
            handler(event.relatedTarget ? event.relatedTarget.dataset : {});
        });
    }

    // Reset every modal form when it closes (restores the server-rendered defaults)
    document.querySelectorAll('.modal').forEach(function (modal) {
        modal.addEventListener('hidden.bs.modal', function () {
            var form = modal.querySelector('form');
            if (form) form.reset();
        });
    });

    onModalShow('editNoteModal', function (data) {
        document.getElementById('editNoteId').value = data.noteId;
        document.getElementById('editNote').value = data.note;
        document.getElementById('editNoteDate').value = data.noteDate;
    });

    // My Tasks multi-selection for the Close/Complete header button
    var taskCheckboxes = Array.prototype.slice.call(document.querySelectorAll('.task-select'));
    var selectAllTasks = document.getElementById('selectAllTasks');

    function selectedTasks() {
        return taskCheckboxes.filter(function (cb) { return cb.checked; });
    }

    function updateTaskSelection() {
        var count = selectedTasks().length;
        document.querySelectorAll('.task-selection-action').forEach(function (btn) {
            btn.disabled = count === 0;
        });
        if (selectAllTasks) {
            selectAllTasks.checked = count > 0 && count === taskCheckboxes.length;
            selectAllTasks.indeterminate = count > 0 && count < taskCheckboxes.length;
        }
    }

    taskCheckboxes.forEach(function (cb) { cb.addEventListener('change', updateTaskSelection); });
    if (selectAllTasks) {
        selectAllTasks.addEventListener('change', function () {
            taskCheckboxes.forEach(function (cb) { cb.checked = selectAllTasks.checked; });
            updateTaskSelection();
        });
    }

    // Copies the selected tasks into a modal's hidden assignment ids and task name list
    function fillSelectedTasks(modal) {
        var inputs = modal.querySelector('.selected-task-inputs');
        inputs.innerHTML = '';
        selectedTasks().forEach(function (cb) {
            var input = document.createElement('input');
            input.type = 'hidden';
            input.name = 'assignmentids[]';
            input.value = cb.value;
            inputs.appendChild(input);
        });
        modal.querySelector('.selected-task-names').textContent =
            selectedTasks().map(function (cb) { return cb.dataset.taskName; }).join(', ');
    }

    onModalShow('closeTaskModal', function () {
        fillSelectedTasks(document.getElementById('closeTaskModal'));
    });

    // Restore the selection state after a back/forward navigation keeps checkboxes checked
    updateTaskSelection();

    // Artefact multi-selection for the Milestone and Remove header buttons
    var artefactCheckboxes = Array.prototype.slice.call(document.querySelectorAll('.artefact-select'));
    var selectAllArtefacts = document.getElementById('selectAllArtefacts');

    function selectedArtefacts() {
        return artefactCheckboxes.filter(function (cb) { return cb.checked; });
    }

    function updateArtefactSelection() {
        var count = selectedArtefacts().length;
        document.querySelectorAll('.artefact-selection-action').forEach(function (btn) {
            btn.disabled = count === 0;
        });
        if (selectAllArtefacts) {
            selectAllArtefacts.checked = count > 0 && count === artefactCheckboxes.length;
            selectAllArtefacts.indeterminate = count > 0 && count < artefactCheckboxes.length;
        }
    }

    artefactCheckboxes.forEach(function (cb) { cb.addEventListener('change', updateArtefactSelection); });
    if (selectAllArtefacts) {
        selectAllArtefacts.addEventListener('change', function () {
            artefactCheckboxes.forEach(function (cb) { cb.checked = selectAllArtefacts.checked; });
            updateArtefactSelection();
        });
    }

    // Copies the current selection into a modal's hidden inputs and title list
    function fillSelectedArtefacts(modal) {
        var inputs = modal.querySelector('.selected-artefact-inputs');
        inputs.innerHTML = '';
        selectedArtefacts().forEach(function (cb) {
            var input = document.createElement('input');
            input.type = 'hidden';
            input.name = 'artefactlinkids[]';
            input.value = cb.value;
            inputs.appendChild(input);
        });
        modal.querySelector('.selected-artefact-titles').textContent =
            selectedArtefacts().map(function (cb) { return cb.dataset.artefactTitle; }).join(', ');
    }

    if (document.getElementById('unlinkArtefactModal')) {
        onModalShow('unlinkArtefactModal', function () {
            fillSelectedArtefacts(document.getElementById('unlinkArtefactModal'));
        });
    }

    if (document.getElementById('associateMilestoneModal')) {
        onModalShow('associateMilestoneModal', function () {
            fillSelectedArtefacts(document.getElementById('associateMilestoneModal'));
            showMilestoneStepDates('');
        });
    }

    // Show only the selected milestone's step dates; hidden fieldsets are disabled so they aren't submitted
    function showMilestoneStepDates(milestoneId) {
        document.querySelectorAll('.milestone-step-dates').forEach(function (fieldset) {
            var match = fieldset.dataset.milestoneId === milestoneId;
            fieldset.classList.toggle('d-none', !match);
            fieldset.disabled = !match;
        });
    }

    var associateMilestoneSelect = document.getElementById('associateMilestoneSelect');
    if (associateMilestoneSelect) {
        associateMilestoneSelect.addEventListener('change', function () {
            showMilestoneStepDates(associateMilestoneSelect.value);
        });
    }

    // Cap the link ratio at the selected artefact's unallocated ratio, and default to it
    var linkArtefactSelect = document.getElementById('linkArtefactSelect');
    if (linkArtefactSelect) {
        linkArtefactSelect.addEventListener('change', function () {
            var ratio = document.getElementById('linkRatio');
            var help = document.getElementById('linkRatioHelp');
            var remaining = linkArtefactSelect.selectedOptions[0].dataset.remainingRatio;
            ratio.max = remaining || 1;
            ratio.value = remaining || '';
            help.textContent = remaining
                ? 'Share of the artefact\'s work done in this activity (up to ' + remaining + ' unallocated).'
                : 'Share of the artefact\'s work done in this activity.';
        });
    }

    // Make artefact and task rows clickable (the checkbox cell only toggles selection)
    document.querySelectorAll('.clickable-row').forEach(function (row) {
        row.addEventListener('click', function (e) {
            if (!e.target.closest('a, button, .dropdown, .row-select')) {
                window.location.href = row.dataset.href;
            }
        });
    });
    </script>
</body>
</html>
