<?php
    session_start();
    require_once '../src/autoload.php';

    if (isset($_POST['activityid']))
        $activityid = $_POST['activityid'];

    $authService = new AuthService();
    $eid = $authService->validateSession();
    
    if (!$eid) {
        header('Location: index.php');
        exit();
    }
    
    $activityService = new ActivityService();
    $artefactService = new ArtefactService();
    
    $message = '';
    $messageType = '';
    
    // Handle note submission
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['add_note'])) {
        $note = trim($_POST['note'] ?? '');
        $notedate = $_POST['notedate'] ?? date('Y-m-d');
        
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
        $notedate = $_POST['notedate'] ?? '';

        if ($noteid && !empty($note) && $notedate) {
            if ($activityService->updateActivityNote($noteid, $eid, $note, $notedate)) {
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
        $parenttaskid = $_POST['parenttaskid'] ?? null;

        // Check if user is activity manager
        if (!$activityService->isActivityManager($activityid, $eid)) {
            $message = 'Only the activity manager can add tasks.';
            $messageType = 'danger';
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

    // Handle task deletion
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['delete_task'])) {
        $taskid = $_POST['taskid'] ?? '';

        // Check if user is activity manager
        if (!$activityService->isActivityManager($activityid, $eid)) {
            $message = 'Only the activity manager can delete tasks.';
            $messageType = 'danger';
        } elseif ($taskid) {
            if ($activityService->deleteActivityTask($taskid)) {
                $message = 'Task deleted successfully!';
                $messageType = 'success';
            } else {
                $message = 'Error deleting task. Tasks with subtasks cannot be deleted.';
                $messageType = 'danger';
            }
        } else {
            $message = 'Please select a task to delete.';
            $messageType = 'warning';
        }
    }

    // Handle task assignment
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['assign_task'])) {
        $taskid = $_POST['assign_taskid'] ?? '';
        $userid = $_POST['assign_userid'] ?? '';
        $assigneddate = $_POST['assigneddate'] ?? date('Y-m-d');

        // Check if user is activity manager
        if (!$activityService->isActivityManager($activityid, $eid)) {
            $message = 'Only the activity manager can assign tasks.';
            $messageType = 'danger';
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
        $actualstart = $_POST['edit_actualstart'] ?? null;
        $actualfinish = $_POST['edit_actualfinish'] ?? null;
        $plannedhours = $_POST['edit_plannedhours'] ?? null;
        $forecasthours = $_POST['edit_forecasthours'] ?? null;

        // Convert empty strings to null for date fields
        $plannedstart = !empty($plannedstart) ? $plannedstart : null;
        $plannedfinish = !empty($plannedfinish) ? $plannedfinish : null;
        $forecaststart = !empty($forecaststart) ? $forecaststart : null;
        $forecastfinish = !empty($forecastfinish) ? $forecastfinish : null;
        $actualstart = !empty($actualstart) ? $actualstart : null;
        $actualfinish = !empty($actualfinish) ? $actualfinish : null;
        $plannedhours = !empty($plannedhours) ? $plannedhours : null;
        $forecasthours = !empty($forecasthours) ? $forecasthours : null;

        // Check if user is project manager
        if (!$activityService->isProjectManager($activityid, $eid)) {
            $message = 'Only the project manager can edit activity details.';
            $messageType = 'danger';
        } elseif (!empty($activityname)) {
            if ($activityService->updateActivity($activityid, $activityname, $activitydescription, $plannedstart, $plannedfinish, $forecaststart, $forecastfinish, $actualstart, $actualfinish, $plannedhours, $forecasthours)) {
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

    // Handle task reassignment
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['reassign_task'])) {
        $assignmentid = $_POST['reassign_assignmentid'] ?? '';
        $newuserid = $_POST['reassign_userid'] ?? '';
        $reassigndate = $_POST['reassigndate'] ?? date('Y-m-d');

        if ($assignmentid && $newuserid) {
            if ($activityService->reassignTask($assignmentid, $newuserid, $reassigndate)) {
                $message = 'Task reassigned successfully!';
                $messageType = 'success';
            } else {
                $message = 'Error reassigning task. The task may already be assigned to the selected user or the assignment may no longer be valid.';
                $messageType = 'danger';
            }
        } else {
            $message = 'Please select a user to reassign the task to.';
            $messageType = 'warning';
        }
    }

    // Handle task closing
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['close_task'])) {
        $assignmentid = $_POST['close_assignmentid'] ?? '';
        $closedate = $_POST['closedate'] ?? date('Y-m-d');

        if ($assignmentid) {
            $result = $activityService->closeTask($assignmentid, $eid, $activityid, $closedate);

            if ($result['success']) {
                $message = $result['message'];
                $messageType = 'success';
            } else {
                $message = $result['message'];
                $messageType = 'danger';
            }
        } else {
            $message = 'Invalid task assignment.';
            $messageType = 'warning';
        }
    }

    // Handle time booking
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['book_time'])) {
        $assignmentid = $_POST['book_assignmentid'] ?? '';
        $hours = $_POST['book_hours'] ?? '';
        $bookdate = $_POST['book_date'] ?? date('Y-m-d');

        // Validate input
        if ($assignmentid && $hours && $bookdate) {
            // Validate hours is a positive number
            if (is_numeric($hours) && $hours > 0) {
                if ($activityService->bookTime($assignmentid, $hours, $bookdate)) {
                    $message = "Successfully logged {$hours} hours for {$bookdate}";
                    $messageType = 'success';
                } else {
                    $message = 'Error booking time. Please try again.';
                    $messageType = 'danger';
                }
            } else {
                $message = 'Please enter a valid number of hours (greater than 0).';
                $messageType = 'warning';
            }
        } else {
            $message = 'Please fill in all required fields for time booking.';
            $messageType = 'warning';
        }
    }
    
    $activitydata = $activityService->getActivityDetails($activityid);
    $activitydata['consumedhours'] = $activityService->getConsumedHours($activityid);
    $taskHeirarchy = $activityService->getTaskHeirarchy($activityid);
    $notelist = $activityService->getActivityNotes($activityid);
    $userTasks = $activityService->getActivityTasksForUser($activityid, $eid);

    // Check if current user is activity manager
    $isActivityManager = $activityService->isActivityManager($activityid, $eid);

    // Check if current user is project manager
    $isProjectManager = $activityService->isProjectManager($activityid, $eid);

    // Get tasks for parent selection dropdown
    $tasksForParentSelection = $activityService->getTasksForParentSelection($activityid);

    // Get data for task assignment
    $unassignedTasks = $activityService->getUnassignedTasks($activityid);
    $allEmployees = $activityService->getAllEmployees();

    $artefactSummary = $artefactService->getArtefactsSummaryForActivity($activityid);
?>
<!DOCTYPE html>
<html lang ="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
	<title>OsoEM - Activity</title>
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.4.1/css/bootstrap.min.css">
    <link rel="stylesheet" href="css/activity.css">
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.7.1/jquery.min.js"></script>
    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.4.1/js/bootstrap.min.js"></script>
</head>
<body>
    <?php include 'includes/header.php'; ?>
    <div class="container">
    
    <?php if ($message): ?>
        <div class="alert alert-<?php echo $messageType; ?> alert-dismissible">
            <a href="#" class="close" data-dismiss="alert" aria-label="close">&times;</a>
            <?php echo htmlspecialchars($message); ?>
        </div>
    <?php endif; ?>

    <div class="row">
        <div class="col-sm-12">
            <h2><small>ACTIVITY</small></h2>
            <h2><?php echo htmlspecialchars($activitydata['Activity_Name']); ?></h2>
        </div>
        <div class="col-sm-6">
            <!-- Activity Details -->
            <div>
                <h4>Activity Description</h4>
                <?php if ($isProjectManager): ?>
                    <button type="button" class="btn btn-primary btn-sm" data-toggle="modal" data-target="#editActivityModal">
                        <span class="glyphicon glyphicon-edit"></span> Edit Activity
                    </button>
                <?php endif; ?>
                <p><?php echo htmlspecialchars($activitydata['Description']); ?> </p>
                <p>Category: <?php echo htmlspecialchars($activitydata['Category']); ?> </p>
                <p>Sub-category: <?php echo htmlspecialchars($activitydata['Subcategory']); ?> </p>
            </div>

            <!-- Activity Manager -->
            <div>
                <h4>Activity Manager</h4>
                <p><?php echo htmlspecialchars($activitydata['Activity_Manager']); ?> </p>
            </div>

            <!-- Gantt Chart -->
            <div>
                <h4>Schedule</h4>
                <div class="well">
                <?php
                // Parse dates and handle null/empty values
                $plannedStart = !empty($activitydata['Planned_Start']) ? new DateTime($activitydata['Planned_Start']) : null;
                $plannedFinish = !empty($activitydata['Planned_Finish']) ? new DateTime($activitydata['Planned_Finish']) : null;
                $forecastStart = !empty($activitydata['Forecast_Start']) ? new DateTime($activitydata['Forecast_Start']) : null;
                $forecastFinish = !empty($activitydata['Forecast_Finish']) ? new DateTime($activitydata['Forecast_Finish']) : null;
                $actualStart = !empty($activitydata['Actual_Start']) ? new DateTime($activitydata['Actual_Start']) : null;
                $actualFinish = !empty($activitydata['Actual_Finish']) ? new DateTime($activitydata['Actual_Finish']) : null;
                $ganttChart = new GanttChart($plannedStart, $plannedFinish, $forecastStart, $forecastFinish, $actualStart, $actualFinish);
                $ganttChart->draw();
                ?>
                </div>
            </div>

            <!-- Hours Chart -->
            <div>
                <h4>Hours</h4>
                <div class="well">
                <?php
                $plannedHours = (float)($activitydata['Planned_Hours'] ?? 0);
                $forecastHours = (float)($activitydata['Forecast_Hours'] ?? 0);
                $consumedHours = (float)($activitydata['consumedhours'] ?? 0);
                $hoursChart = new HoursChart($plannedHours,$forecastHours,$consumedHours);
                $hoursChart->draw();
                ?>
                </div>
            </div>
        </div>

        <div class="col-sm-6">
            <!-- Task Tree -->
            <h4>Tasks</h4>
            <div class="well">
                <?php if ($isActivityManager): ?>
                    <div>
                        <button type="button" class="btn btn-success btn-sm" data-toggle="modal" data-target="#addTaskModal">
                            <span class="glyphicon glyphicon-plus"></span> Add Task
                        </button>
                        <button type="button" class="btn btn-info btn-sm" data-toggle="modal" data-target="#assignTaskModal">
                            <span class="glyphicon glyphicon-user"></span> Assign Task
                        </button>
                        <button type="button" class="btn btn-danger btn-sm" data-toggle="modal" data-target="#deleteTaskModal">
                            <span class="glyphicon glyphicon-trash"></span> Delete Task
                        </button>
                    </div>
                <?php endif; ?>
                <?php
                $taskTree = new TaskTree($taskHeirarchy,$activitydata['Activity_Name']);
                $taskTree->draw();
                ?>
            </div>
        </div>
    </div>  <!--/row-->

    <div class="row">
        <!-- My Tasks -->
        <div class="col-sm-12">
            <h3>My Tasks</h3>
            <?php if (!empty($userTasks)): ?>
                <table class="table table-striped table-hover">
                    <thead>
                        <tr>
                            <th>Task Name</th>
                            <th>Status</th>
                            <th>Assigned Date</th>
                            <th>Booked Hours</th>
                            <th>Actions</th>
                        </tr>
                    </thead>
                    <tbody>
                        <?php foreach ($userTasks as $task): ?>
                            <tr>
                                <td><?php echo htmlspecialchars($task['Task_Hierarchy']); ?></td>
                                <td>
                                    <span class="label label-warning">
                                        <?php echo htmlspecialchars($task['Status']); ?>
                                    </span>
                                </td>
                                <td><?php echo htmlspecialchars($task['Assigned_Date']); ?></td>
                                <td><?php echo htmlspecialchars($task['Booked_Hours']); ?></td>
                                <td>
                                    <button type="button" class="btn btn-warning btn-xs"
                                            data-toggle="modal"
                                            data-target="#reassignTaskModal"
                                            onclick="openReassignModal(<?php echo $task['assignmentid']; ?>, '<?php echo htmlspecialchars(addslashes($task['Task_Hierarchy']), ENT_QUOTES); ?>')"
                                            title="Reassign this task">
                                        <span class="glyphicon glyphicon-transfer"></span> Reassign
                                    </button>
                                    <button type="button" class="btn btn-<?php echo $isActivityManager ? 'success' : 'info'; ?> btn-xs"
                                            data-toggle="modal"
                                            data-target="#closeTaskModal"
                                            onclick="openCloseModal(<?php echo $task['assignmentid']; ?>, '<?php echo htmlspecialchars(addslashes($task['Task_Hierarchy']), ENT_QUOTES); ?>', <?php echo $isActivityManager ? 'true' : 'false'; ?>)"
                                            title="<?php echo $isActivityManager ? 'Close this task' : 'Complete and reassign to activity manager'; ?>">
                                        <span class="glyphicon glyphicon-<?php echo $isActivityManager ? 'ok' : 'share'; ?>"></span> <?php echo $isActivityManager ? 'Close' : 'Complete'; ?>
                                    </button>
                                    <button type="button" class="btn btn-primary btn-xs"
                                            data-toggle="modal"
                                            data-target="#bookTimeModal"
                                            onclick="openBookTimeModal(<?php echo $task['assignmentid']; ?>, '<?php echo htmlspecialchars(addslashes($task['Task_Hierarchy']), ENT_QUOTES); ?>')"
                                            title="Book time for this task">
                                        <span class="glyphicon glyphicon-time"></span> Book Time
                                    </button>
                                </td>
                            </tr>
                        <?php endforeach; ?>
                    </tbody>
                </table>
            <?php else: ?>
                <p class="text-muted">No open tasks assigned to you in this activity.</p>
            <?php endif; ?>
        </div>

        <!-- Artefacts -->
        <div class="col-sm-12">
            <h3>Artefacts</h3>
            <?php if (!empty($artefactSummary)): ?>
                <?php
                $currentArtefactType = '';
                $artefactsByType = [];

                // Group artefacts by type
                foreach ($artefactSummary as $artefact) {
                    $artefactsByType[$artefact['Artefact']][] = $artefact;
                }

                $artefactTypes = array_keys($artefactsByType);
                $chunkedArtefacts = array_chunk($artefactTypes, 2, true);
                ?>

                <?php foreach ($chunkedArtefacts as $artefactTypeChunk): ?>
                    <div class="row">
                        <?php foreach ($artefactTypeChunk as $artefactType): ?>
                            <div class="col-sm-12">
                                <div class="panel panel-default artefact-panel">
                                    <div class="panel-heading">
                                        <h4 class="panel-title"><?php echo htmlspecialchars($artefactType); ?></h4>
                                    </div>
                                    <div class="panel-body">
                                        <table class="table table-striped table-hover artefact-table">
                                            <tbody>
                                                <?php foreach ($artefactsByType[$artefactType] as $artefact): ?>
                                                    <tr class="artefact-row <?php echo $artefact['Status'] == 'Complete' ? 'success' : ''; ?>"
                                                        onclick="navigateToArtefact(<?php echo $artefact['artefactlinkid']; ?>)"
                                                        style="cursor: pointer;">
                                                        <td><?php echo htmlspecialchars($artefact['Title']); ?></td>
                                                        <td>
                                                            <span class="label <?php echo $artefact['Status'] == 'Complete' ? 'label-success' : 'label-warning'; ?>">
                                                                <?php echo htmlspecialchars($artefact['Status']); ?>
                                                            </span>
                                                        </td>
                                                        <td>
                                                            <div class="progress">
                                                                <div class="progress-bar <?php echo $artefact['Progress'] == 100 ? 'progress-bar-custom-complete' : 'progress-bar-custom-progress'; ?>"
                                                                    role="progressbar"
                                                                     style="width: <?php echo $artefact['Progress']; ?>%">
                                                                    <?php echo number_format($artefact['Progress'], 0); ?>%
                                                                </div>
                                                            </div>
                                                        </td>
                                                    </tr>
                                                <?php endforeach; ?>
                                            </tbody>
                                        </table>
                                    </div>
                                </div>
                            </div>
                        <?php endforeach; ?>
                    </div>
                <?php endforeach; ?>
            <?php else: ?>
                <p class="text-muted">No artefacts associated with this activity.</p>
            <?php endif; ?>
        </div>

        <!-- Activity Notes -->
        <div class="col-sm-12">
            <h3>Notes</h3>
                <div class="pull-right">
                    <button type="button" class="btn btn-primary btn-sm" data-toggle="modal" data-target="#addNoteModal">
                        <span class="glyphicon glyphicon-plus"></span> Add Note
                    </button>
                </div>
            </h4>
            <div style="clear: both;">
                <?php if (!empty($notelist)): ?>
                    <?php foreach ($notelist as $note): ?>
                        <div class="note-item" style="position: relative; border-bottom: 1px solid #eee; padding-bottom: 10px; margin-bottom: 10px;">
                            <div class="note-text" >
                                <?php echo htmlspecialchars($note['Note']); ?>
                            </div>
                            <div class="note-meta" style="display: flex; justify-content: space-between; align-items: center; margin-top: 5px;">
                                <span>
                                    By <?php echo htmlspecialchars($note['Note_By']); ?> on <?php echo htmlspecialchars($note['Note_Date']); ?>
                                </span>
                                <?php if ($note['User_ID'] == $eid): ?>
                                    <div style="display: flex; gap: 5px;">
                                        <button type="button" class="btn btn-xs btn-info"
                                                title="Edit this note"
                                                onclick="openEditModal(<?php echo $note['Note_ID']; ?>, '<?php echo htmlspecialchars(addslashes($note['Note']), ENT_QUOTES); ?>', '<?php echo $note['Note_Date']; ?>')">
                                            <span class="glyphicon glyphicon-edit"></span>
                                        </button>
                                        <form method="POST" style="margin: 0; display: inline;"
                                              onsubmit="return confirm('Are you sure you want to delete this note?');">
                                            <input type="hidden" name="noteid" value="<?php echo $note['Note_ID']; ?>">
                                            <input type="hidden" name="activityid" value="<?php echo $activityid; ?>">
                                            <button type="submit" name="delete_note" class="btn btn-xs btn-danger"
                                                    title="Delete this note">
                                                <span class="glyphicon glyphicon-trash"></span>
                                            </button>
                                        </form>
                                    </div>
                                <?php endif; ?>
                            </div>
                        </div>
                    <?php endforeach; ?>
                <?php else: ?>
                    <p class="text-muted">No notes available for this activity.</p>
                <?php endif; ?>
            </div>
        </div>
    </div>  <!--row-->

    </div>
    <!-- Add Note Modal -->
    <div class="modal fade" id="addNoteModal" tabindex="-1" role="dialog" aria-labelledby="addNoteModalLabel">
        <div class="modal-dialog" role="document">
            <div class="modal-content">
                <form method="POST">
                    <input type="hidden" name="activityid" value="<?php echo $activityid; ?>">
                    <div class="modal-header">
                        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
                            <span aria-hidden="true">&times;</span>
                        </button>
                        <h4 class="modal-title" id="addNoteModalLabel">Add Activity Note</h4>
                    </div>
                    <div class="modal-body">
                        <div class="form-group">
                            <label for="notedate">Date:</label>
                            <input type="date" class="form-control" id="notedate" name="notedate" 
                                   value="<?php echo date('Y-m-d'); ?>" required>
                        </div>
                        <div class="form-group">
                            <label for="note">Note:</label>
                            <textarea class="form-control" id="note" name="note" rows="4" 
                                      placeholder="Enter your note here..." required></textarea>
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
                        <button type="submit" name="add_note" class="btn btn-primary">Add Note</button>
                    </div>
                </form>
            </div>
        </div>
    </div>
    
    <!-- Edit Note Modal -->
    <div class="modal fade" id="editNoteModal" tabindex="-1" role="dialog" aria-labelledby="editNoteModalLabel">
        <div class="modal-dialog" role="document">
            <div class="modal-content">
                <form method="POST">
                    <input type="hidden" name="activityid" value="<?php echo $activityid; ?>">
                    <input type="hidden" name="noteid" id="editNoteId">
                    <div class="modal-header">
                        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
                            <span aria-hidden="true">&times;</span>
                        </button>
                        <h4 class="modal-title" id="editNoteModalLabel">Edit Activity Note</h4>
                    </div>
                    <div class="modal-body">
                        <div class="form-group">
                            <label for="editNoteDate">Date:</label>
                            <input type="date" class="form-control" id="editNoteDate" name="notedate" required>
                        </div>
                        <div class="form-group">
                            <label for="editNote">Note:</label>
                            <textarea class="form-control" id="editNote" name="note" rows="4"
                                      placeholder="Enter your note here..." required></textarea>
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
                        <button type="submit" name="edit_note" class="btn btn-primary">Update Note</button>
                    </div>
                </form>
            </div>
        </div>
    </div>

    <!-- Add Task Modal -->
    <div class="modal fade" id="addTaskModal" tabindex="-1" role="dialog" aria-labelledby="addTaskModalLabel">
        <div class="modal-dialog" role="document">
            <div class="modal-content">
                <form method="POST">
                    <input type="hidden" name="activityid" value="<?php echo $activityid; ?>">
                    <div class="modal-header">
                        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
                            <span aria-hidden="true">&times;</span>
                        </button>
                        <h4 class="modal-title" id="addTaskModalLabel">Add Task</h4>
                    </div>
                    <div class="modal-body">
                        <div class="form-group">
                            <label for="taskname">Task Name:</label>
                            <input type="text" class="form-control" id="taskname" name="taskname"
                                   placeholder="Enter task name..." required>
                        </div>
                        <div class="form-group">
                            <label for="parenttaskid">Parent Task:</label>
                            <select class="form-control" id="parenttaskid" name="parenttaskid">
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
                        <button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
                        <button type="submit" name="add_task" class="btn btn-success">Add Task</button>
                    </div>
                </form>
            </div>
        </div>
    </div>

    <!-- Delete Task Modal -->
    <div class="modal fade" id="deleteTaskModal" tabindex="-1" role="dialog" aria-labelledby="deleteTaskModalLabel">
        <div class="modal-dialog" role="document">
            <div class="modal-content">
                <form method="POST">
                    <input type="hidden" name="activityid" value="<?php echo $activityid; ?>">
                    <div class="modal-header">
                        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
                            <span aria-hidden="true">&times;</span>
                        </button>
                        <h4 class="modal-title" id="deleteTaskModalLabel">Delete Task</h4>
                    </div>
                    <div class="modal-body">
                        <div class="alert alert-warning">
                            <strong>Warning:</strong> This action cannot be undone. Tasks with subtasks cannot be deleted.
                        </div>
                        <div class="form-group">
                            <label for="deleteTaskSelect">Select Task to Delete:</label>
                            <select class="form-control" id="deleteTaskSelect" name="taskid" required>
                                <option value="">Please select a task...</option>
                                <?php foreach ($tasksForParentSelection as $task): ?>
                                    <option value="<?php echo $task['taskid']; ?>">
                                        <?php echo htmlspecialchars($task['taskname']); ?>
                                    </option>
                                <?php endforeach; ?>
                            </select>
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
                        <button type="submit" name="delete_task" class="btn btn-danger"
                                onclick="return confirm('Are you sure you want to delete this task?');">
                            Delete Task
                        </button>
                    </div>
                </form>
            </div>
        </div>
    </div>

    <!-- Assign Task Modal -->
    <div class="modal fade" id="assignTaskModal" tabindex="-1" role="dialog" aria-labelledby="assignTaskModalLabel">
        <div class="modal-dialog" role="document">
            <div class="modal-content">
                <form method="POST">
                    <input type="hidden" name="activityid" value="<?php echo $activityid; ?>">
                    <div class="modal-header">
                        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
                            <span aria-hidden="true">&times;</span>
                        </button>
                        <h4 class="modal-title" id="assignTaskModalLabel">Assign Task</h4>
                    </div>
                    <div class="modal-body">
                        <?php if (!empty($unassignedTasks)): ?>
                            <div class="form-group">
                                <label for="assignTaskSelect">Select Unassigned Task:</label>
                                <select class="form-control" id="assignTaskSelect" name="assign_taskid" required>
                                    <option value="">Please select a task...</option>
                                    <?php foreach ($unassignedTasks as $task): ?>
                                        <option value="<?php echo $task['taskid']; ?>">
                                            <?php echo htmlspecialchars($task['taskname']); ?>
                                        </option>
                                    <?php endforeach; ?>
                                </select>
                            </div>
                            <div class="form-group">
                                <label for="assignUserSelect">Assign to User:</label>
                                <select class="form-control" id="assignUserSelect" name="assign_userid" required>
                                    <option value="">Please select a user...</option>
                                    <?php foreach ($allEmployees as $employee): ?>
                                        <option value="<?php echo $employee['employeeid']; ?>">
                                            <?php echo htmlspecialchars($employee['fullname']); ?>
                                        </option>
                                    <?php endforeach; ?>
                                </select>
                            </div>
                            <div class="form-group">
                                <label for="assigneddate">Assignment Date:</label>
                                <input type="date" class="form-control" id="assigneddate" name="assigneddate"
                                       value="<?php echo date('Y-m-d'); ?>" required>
                            </div>
                        <?php else: ?>
                            <div class="alert alert-info">
                                <strong>Info:</strong> No unassigned tasks available. All tasks in this activity are currently assigned.
                            </div>
                        <?php endif; ?>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
                        <?php if (!empty($unassignedTasks)): ?>
                            <button type="submit" name="assign_task" class="btn btn-info">Assign Task</button>
                        <?php endif; ?>
                    </div>
                </form>
            </div>
        </div>
    </div>

    <!-- Edit Activity Modal -->
    <div class="modal fade" id="editActivityModal" tabindex="-1" role="dialog" aria-labelledby="editActivityModalLabel">
        <div class="modal-dialog modal-lg" role="document">
            <div class="modal-content">
                <form method="POST">
                    <input type="hidden" name="activityid" value="<?php echo $activityid; ?>">
                    <div class="modal-header">
                        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
                            <span aria-hidden="true">&times;</span>
                        </button>
                        <h4 class="modal-title" id="editActivityModalLabel">Edit Activity Details</h4>
                    </div>
                    <div class="modal-body">
                        <div class="row">
                            <div class="col-sm-6">
                                <div class="form-group">
                                    <label for="edit_activityname">Activity Name:</label>
                                    <input type="text" class="form-control" id="edit_activityname" name="edit_activityname"
                                           value="<?php echo htmlspecialchars($activitydata['Activity_Name']); ?>" required>
                                </div>
                                <div class="form-group">
                                    <label for="edit_activitydescription">Description:</label>
                                    <textarea class="form-control" id="edit_activitydescription" name="edit_activitydescription" rows="3"><?php echo htmlspecialchars($activitydata['Description']); ?></textarea>
                                </div>
                                <div class="form-group">
                                    <label for="edit_plannedstart">Planned Start:</label>
                                    <input type="date" class="form-control" id="edit_plannedstart" name="edit_plannedstart"
                                           value="<?php echo $activitydata['Planned_Start']; ?>">
                                </div>
                                <div class="form-group">
                                    <label for="edit_plannedfinish">Planned Finish:</label>
                                    <input type="date" class="form-control" id="edit_plannedfinish" name="edit_plannedfinish"
                                           value="<?php echo $activitydata['Planned_Finish']; ?>">
                                </div>
                                <div class="form-group">
                                    <label for="edit_plannedhours">Planned Hours:</label>
                                    <input type="number" step="0.01" class="form-control" id="edit_plannedhours" name="edit_plannedhours"
                                           value="<?php echo $activitydata['Planned_Hours']; ?>">
                                </div>
                            </div>
                            <div class="col-sm-6">
                                <div class="form-group">
                                    <label for="edit_forecaststart">Forecast Start:</label>
                                    <input type="date" class="form-control" id="edit_forecaststart" name="edit_forecaststart"
                                           value="<?php echo $activitydata['Forecast_Start']; ?>">
                                </div>
                                <div class="form-group">
                                    <label for="edit_forecastfinish">Forecast Finish:</label>
                                    <input type="date" class="form-control" id="edit_forecastfinish" name="edit_forecastfinish"
                                           value="<?php echo $activitydata['Forecast_Finish']; ?>">
                                </div>
                                <div class="form-group">
                                    <label for="edit_forecasthours">Forecast Hours:</label>
                                    <input type="number" step="0.01" class="form-control" id="edit_forecasthours" name="edit_forecasthours"
                                           value="<?php echo $activitydata['Forecast_Hours']; ?>">
                                </div>
                                <div class="form-group">
                                    <label for="edit_actualstart">Actual Start:</label>
                                    <input type="date" class="form-control" id="edit_actualstart" name="edit_actualstart"
                                           value="<?php echo $activitydata['Actual_Start']; ?>">
                                </div>
                                <div class="form-group">
                                    <label for="edit_actualfinish">Actual Finish:</label>
                                    <input type="date" class="form-control" id="edit_actualfinish" name="edit_actualfinish"
                                           value="<?php echo $activitydata['Actual_Finish']; ?>">
                                </div>
                            </div>
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
                        <button type="submit" name="edit_activity" class="btn btn-primary">Update Activity</button>
                    </div>
                </form>
            </div>
        </div>
    </div>

    <!-- Reassign Task Modal -->
    <div class="modal fade" id="reassignTaskModal" tabindex="-1" role="dialog" aria-labelledby="reassignTaskModalLabel">
        <div class="modal-dialog" role="document">
            <div class="modal-content">
                <form method="POST">
                    <input type="hidden" name="activityid" value="<?php echo $activityid; ?>">
                    <input type="hidden" name="reassign_assignmentid" id="reassignAssignmentId">
                    <div class="modal-header">
                        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
                            <span aria-hidden="true">&times;</span>
                        </button>
                        <h4 class="modal-title" id="reassignTaskModalLabel">Reassign Task</h4>
                    </div>
                    <div class="modal-body">
                        <div class="alert alert-info">
                            <strong>Task:</strong> <span id="reassignTaskName"></span>
                        </div>
                        <div class="form-group">
                            <label for="reassignUserSelect">Reassign to User:</label>
                            <select class="form-control" id="reassignUserSelect" name="reassign_userid" required>
                                <option value="">Please select a user...</option>
                                <?php foreach ($allEmployees as $employee): ?>
                                    <option value="<?php echo $employee['employeeid']; ?>">
                                        <?php echo htmlspecialchars($employee['fullname']); ?>
                                    </option>
                                <?php endforeach; ?>
                            </select>
                        </div>
                        <div class="form-group">
                            <label for="reassigndate">Reassignment Date:</label>
                            <input type="date" class="form-control" id="reassigndate" name="reassigndate"
                                   value="<?php echo date('Y-m-d'); ?>" required>
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
                        <button type="submit" name="reassign_task" class="btn btn-warning">Reassign Task</button>
                    </div>
                </form>
            </div>
        </div>
    </div>

    <!-- Close Task Modal -->
    <div class="modal fade" id="closeTaskModal" tabindex="-1" role="dialog" aria-labelledby="closeTaskModalLabel">
        <div class="modal-dialog" role="document">
            <div class="modal-content">
                <form method="POST">
                    <input type="hidden" name="activityid" value="<?php echo $activityid; ?>">
                    <input type="hidden" name="close_assignmentid" id="closeAssignmentId">
                    <div class="modal-header">
                        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
                            <span aria-hidden="true">&times;</span>
                        </button>
                        <h4 class="modal-title" id="closeTaskModalLabel">Close Task</h4>
                    </div>
                    <div class="modal-body">
                        <div class="alert alert-info">
                            <strong>Task:</strong> <span id="closeTaskName"></span>
                        </div>
                        <div class="alert" id="closeTaskAction">
                            <strong>Action:</strong> <span id="closeActionDescription"></span>
                        </div>
                        <div class="form-group">
                            <label for="closedate">Date:</label>
                            <input type="date" class="form-control" id="closedate" name="closedate"
                                   value="<?php echo date('Y-m-d'); ?>" required>
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
                        <button type="submit" name="close_task" class="btn" id="closeTaskSubmitBtn">Confirm</button>
                    </div>
                </form>
            </div>
        </div>
    </div>

    <!-- Book Time Modal -->
    <div class="modal fade" id="bookTimeModal" tabindex="-1" role="dialog" aria-labelledby="bookTimeModalLabel">
        <div class="modal-dialog" role="document">
            <div class="modal-content">
                <form method="POST">
                    <input type="hidden" name="activityid" value="<?php echo $activityid; ?>">
                    <input type="hidden" name="book_assignmentid" id="bookAssignmentId">
                    <div class="modal-header">
                        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
                            <span aria-hidden="true">&times;</span>
                        </button>
                        <h4 class="modal-title" id="bookTimeModalLabel">Book Time</h4>
                    </div>
                    <div class="modal-body">
                        <div class="alert alert-info">
                            <strong>Task:</strong> <span id="bookTimeTaskName"></span>
                        </div>
                        <div class="form-group">
                            <label for="book_date">Date:</label>
                            <input type="date" class="form-control" id="book_date" name="book_date"
                                   value="<?php echo date('Y-m-d'); ?>" required>
                        </div>
                        <div class="form-group">
                            <label for="book_hours">Hours:</label>
                            <input type="number" class="form-control" id="book_hours" name="book_hours"
                                   min="0.25" max="24" step="0.25" placeholder="e.g., 2.5" required>
                            <small class="text-muted">Enter hours worked (minimum 0.25, maximum 24)</small>
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>
                        <button type="submit" name="book_time" class="btn btn-primary">Book Time</button>
                    </div>
                </form>
            </div>
        </div>
    </div>
	</div>	
	
	<script>
	function openEditModal(noteId, noteText, noteDate) {
	    document.getElementById('editNoteId').value = noteId;
	    document.getElementById('editNote').value = noteText;
	    document.getElementById('editNoteDate').value = noteDate;
	    $('#editNoteModal').modal('show');
	}

	function navigateToArtefact(artefactlinkid) {
	    // Create a form and submit it to artefact.php
	    var form = document.createElement('form');
	    form.method = 'POST';
	    form.action = 'artefact.php';

	    var input = document.createElement('input');
	    input.type = 'hidden';
	    input.name = 'artefactlinkid';
	    input.value = artefactlinkid;

	    form.appendChild(input);
	    document.body.appendChild(form);
	    form.submit();
	}

	// Clear form when Add Task modal is closed
	$('#addTaskModal').on('hidden.bs.modal', function () {
	    $(this).find('form')[0].reset();
	});

	// Clear form when Delete Task modal is closed
	$('#deleteTaskModal').on('hidden.bs.modal', function () {
	    $(this).find('form')[0].reset();
	});

	// Clear form when Assign Task modal is closed
	$('#assignTaskModal').on('hidden.bs.modal', function () {
	    $(this).find('form')[0].reset();
	    // Reset assignment date to today
	    document.getElementById('assigneddate').value = '<?php echo date('Y-m-d'); ?>';
	});

	// Clear form when Edit Activity modal is closed
	$('#editActivityModal').on('hidden.bs.modal', function () {
	    $(this).find('form')[0].reset();
	    // Restore original values
	    document.getElementById('edit_activityname').value = '<?php echo htmlspecialchars($activitydata['Activity_Name'], ENT_QUOTES); ?>';
	    document.getElementById('edit_activitydescription').value = '<?php echo htmlspecialchars($activitydata['Description'], ENT_QUOTES); ?>';
	    document.getElementById('edit_plannedstart').value = '<?php echo $activitydata['Planned_Start']; ?>';
	    document.getElementById('edit_plannedfinish').value = '<?php echo $activitydata['Planned_Finish']; ?>';
	    document.getElementById('edit_forecaststart').value = '<?php echo $activitydata['Forecast_Start']; ?>';
	    document.getElementById('edit_forecastfinish').value = '<?php echo $activitydata['Forecast_Finish']; ?>';
	    document.getElementById('edit_actualstart').value = '<?php echo $activitydata['Actual_Start']; ?>';
	    document.getElementById('edit_actualfinish').value = '<?php echo $activitydata['Actual_Finish']; ?>';
	    document.getElementById('edit_plannedhours').value = '<?php echo $activitydata['Planned_Hours']; ?>';
	    document.getElementById('edit_forecasthours').value = '<?php echo $activitydata['Forecast_Hours']; ?>';
	});

	// Function to open reassign modal with task information
	function openReassignModal(assignmentId, taskName) {
	    document.getElementById('reassignAssignmentId').value = assignmentId;
	    document.getElementById('reassignTaskName').textContent = taskName;

	    // Reset form
	    document.getElementById('reassignUserSelect').value = '';
	    document.getElementById('reassigndate').value = '<?php echo date('Y-m-d'); ?>';
	}

	// Clear form when Reassign Task modal is closed
	$('#reassignTaskModal').on('hidden.bs.modal', function () {
	    $(this).find('form')[0].reset();
	    document.getElementById('reassignAssignmentId').value = '';
	    document.getElementById('reassignTaskName').textContent = '';
	    document.getElementById('reassigndate').value = '<?php echo date('Y-m-d'); ?>';
	});

	// Function to open close task modal with task information
	function openCloseModal(assignmentId, taskName, isActivityManager) {
	    document.getElementById('closeAssignmentId').value = assignmentId;
	    document.getElementById('closeTaskName').textContent = taskName;
	    document.getElementById('closedate').value = '<?php echo date('Y-m-d'); ?>';

	    // Update modal content based on user role
	    var actionAlert = document.getElementById('closeTaskAction');
	    var actionDescription = document.getElementById('closeActionDescription');
	    var submitBtn = document.getElementById('closeTaskSubmitBtn');
	    var modalTitle = document.getElementById('closeTaskModalLabel');

	    if (isActivityManager) {
	        modalTitle.textContent = 'Close Task';
	        actionAlert.className = 'alert alert-success';
	        actionDescription.textContent = 'This will mark the task as completed and close it permanently.';
	        submitBtn.className = 'btn btn-success';
	        submitBtn.textContent = 'Close Task';
	    } else {
	        modalTitle.textContent = 'Complete Task';
	        actionAlert.className = 'alert alert-warning';
	        actionDescription.textContent = 'This will complete your assignment and reassign the task to the activity manager.';
	        submitBtn.className = 'btn btn-info';
	        submitBtn.textContent = 'Complete Task';
	    }
	}

	// Clear form when Close Task modal is closed
	$('#closeTaskModal').on('hidden.bs.modal', function () {
	    $(this).find('form')[0].reset();
	    document.getElementById('closeAssignmentId').value = '';
	    document.getElementById('closeTaskName').textContent = '';
	    document.getElementById('closedate').value = '<?php echo date('Y-m-d'); ?>';
	});

	// Function to open book time modal with task information
	function openBookTimeModal(assignmentId, taskName) {
	    document.getElementById('bookAssignmentId').value = assignmentId;
	    document.getElementById('bookTimeTaskName').textContent = taskName;
	    document.getElementById('book_date').value = '<?php echo date('Y-m-d'); ?>';
	    document.getElementById('book_hours').value = '';
	}

	// Clear form when Book Time modal is closed
	$('#bookTimeModal').on('hidden.bs.modal', function () {
	    $(this).find('form')[0].reset();
	    document.getElementById('bookAssignmentId').value = '';
	    document.getElementById('bookTimeTaskName').textContent = '';
	    document.getElementById('book_date').value = '<?php echo date('Y-m-d'); ?>';
	    document.getElementById('book_hours').value = '';
	});
	</script>
</body>
</html>

