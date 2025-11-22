<?php
    session_start();
    require_once '../src/autoload.php';
    
    $authService = new AuthService();
    $eid = $authService->validateSession();
    
    if (!$eid) {
        header('Location: index.php');
        exit();
    }
    
    $activityService = new ActivityService();
    $message = '';
    $messageType = '';
    
    // Handle form submission
    if ($_SERVER['REQUEST_METHOD'] === 'POST') {
        $assignmentid = $_POST['assignmentid'] ?? '';
        $hours = $_POST['hours'] ?? '';
        $bookeddate = $_POST['bookeddate'] ?? '';
        
        if ($assignmentid && $hours && $bookeddate) {
            if (is_numeric($hours) && $hours > 0) {
                if ($activityService->bookTime($assignmentid, $hours, $bookeddate)) {
                    $message = 'Time booked successfully!';
                    $messageType = 'success';
                } else {
                    $message = 'Error booking time. Please try again.';
                    $messageType = 'danger';
                }
            } else {
                $message = 'Please enter a valid number of hours.';
                $messageType = 'warning';
            }
        } else {
            $message = 'Please fill in all fields.';
            $messageType = 'warning';
        }
    }
    
    $tasks = $activityService->getUserTasksForTimeBooking($eid);
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Osoprom - Book Time</title>
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.4.1/css/bootstrap.min.css">
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.7.1/jquery.min.js"></script>
    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.4.1/js/bootstrap.min.js"></script>
</head>
<body>
    <?php include 'includes/header.php'; ?>
    <div class="container">
        <h2>Book Time</h2>
        
        <?php if ($message): ?>
            <div class="alert alert-<?php echo $messageType; ?> alert-dismissible">
                <a href="#" class="close" data-dismiss="alert" aria-label="close">&times;</a>
                <?php echo htmlspecialchars($message); ?>
            </div>
        <?php endif; ?>
        
        <?php if (empty($tasks)): ?>
            <div class="alert alert-info">
                <strong>No tasks assigned!</strong> You don't have any open task assignments to book time against.
            </div>
        <?php else: ?>
            <form method="POST" class="form-horizontal">
                <div class="form-group">
                    <label class="control-label col-sm-2" for="assignmentid">Task:</label>
                    <div class="col-sm-10">
                        <select class="form-control" id="assignmentid" name="assignmentid" required>
                            <option value="">Select a task...</option>
                            <?php foreach ($tasks as $task): ?>
                                <option value="<?php echo $task['assignmentid']; ?>">
                                    <?php echo htmlspecialchars($task['Project_Name'] . ' - ' . $task['Activity_Name'] . ' - ' . $task['Task_Hierarchy']); ?>
                                    (<?php echo $task['Booked_Hours']; ?> hours booked)
                                </option>
                            <?php endforeach; ?>
                        </select>
                    </div>
                </div>
                
                <div class="form-group">
                    <label class="control-label col-sm-2" for="hours">Hours:</label>
                    <div class="col-sm-10">
                        <input type="number" class="form-control" id="hours" name="hours" 
                               min="0.5" step="0.5" placeholder="Enter hours worked" required>
                    </div>
                </div>
                
                <div class="form-group">
                    <label class="control-label col-sm-2" for="bookeddate">Date:</label>
                    <div class="col-sm-10">
                        <input type="date" class="form-control" id="bookeddate" name="bookeddate" 
                               value="<?php echo date('Y-m-d'); ?>" required>
                    </div>
                </div>
                
                <div class="form-group">
                    <div class="col-sm-offset-2 col-sm-10">
                        <button type="submit" class="btn btn-primary">Book Time</button>
                        <a href="dashboard.php" class="btn btn-default">Cancel</a>
                    </div>
                </div>
            </form>
        <?php endif; ?>
        
        <?php if (!empty($tasks)): ?>
            <hr>
            <h3>My Current Task Assignments</h3>
            <table class="table table-hover">
                <thead>
                    <tr>
                        <th>Project</th>
                        <th>Activity</th>
                        <th>Task</th>
                        <th>Hours Booked</th>
                        <th>Status</th>
                    </tr>
                </thead>
                <tbody>
                    <?php foreach ($tasks as $task): ?>
                        <tr>
                            <td><?php echo htmlspecialchars($task['Project_Name']); ?></td>
                            <td><?php echo htmlspecialchars($task['Activity_Name']); ?></td>
                            <td><?php echo htmlspecialchars($task['Task_Hierarchy']); ?></td>
                            <td><?php echo $task['Booked_Hours']; ?></td>
                            <td>
                                <span class="label label-<?php echo $task['Status'] == 'Open' ? 'success' : 'default'; ?>">
                                    <?php echo $task['Status']; ?>
                                </span>
                            </td>
                        </tr>
                    <?php endforeach; ?>
                </tbody>
            </table>
        <?php endif; ?>
    </div>
</body>
</html>