<?php
    require_once '../src/bootstrap.php';

    $activityService = new ActivityService();
    $message = '';
    $messageType = '';
    
    // Handle form submission
    if ($_SERVER['REQUEST_METHOD'] === 'POST') {
        if (!Csrf::validate($_POST['csrf_token'] ?? '')) {
            http_response_code(403);
            die('Invalid or expired form submission (CSRF token mismatch). Please refresh the page and try again.');
        }

        $assignmentid = (int)($_POST['assignmentid'] ?? 0);
        $hours = $_POST['hours'] ?? '';
        $bookeddate = $_POST['bookeddate'] ?? '';
        $ownAssignmentIds = array_map('intval', array_column($activityService->getUserTasksForTimeBooking($eid), 'assignmentid'));

        if (!$assignmentid || $hours === '' || !$bookeddate) {
            $message = 'Please fill in all fields.';
            $messageType = 'warning';
        } elseif (!in_array($assignmentid, $ownAssignmentIds, true)) {
            $message = 'You can only book time against your own open task assignments.';
            $messageType = 'danger';
        } elseif (!is_numeric($hours) || (float)$hours <= 0 || round((float)$hours, 2) != (float)$hours) {
            $message = 'Please enter a positive number of hours with at most two decimal places.';
            $messageType = 'warning';
        } elseif ($activityService->bookTime($assignmentid, $eid, (float)$hours, $bookeddate)) {
            $message = 'Time booked successfully!';
            $messageType = 'success';
        } else {
            $message = 'Error booking time. Please try again.';
            $messageType = 'danger';
        }
    }

    $tasks = $activityService->getUserTasksForTimeBooking($eid);

    $pageTitle = 'Book Time';
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
                        <li class="breadcrumb-item active" aria-current="page">Book Time</li>
                    </ol>
                </nav>
                <h1 class="h3">Book Time</h1>
                <p class="page-subtitle">Record hours against your open task assignments.</p>
            </div>
        </div>

        <?php if ($message): ?>
            <div class="alert alert-<?php echo $messageType; ?> alert-dismissible fade show" role="alert">
                <?php echo htmlspecialchars($message); ?>
                <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Close"></button>
            </div>
        <?php endif; ?>

        <?php if (empty($tasks)): ?>
            <div class="card">
                <div class="empty-state">
                    <i class="bi bi-clipboard-check"></i>
                    You don't have any open task assignments to book time against.
                </div>
            </div>
        <?php else: ?>
            <div class="card">
                <div class="card-header">
                    <h2 class="card-title">My current task assignments</h2>
                    <div class="card-actions">
                        <button type="button" class="btn btn-sm btn-primary" data-bs-toggle="modal" data-bs-target="#bookTimeModal">
                            <i class="bi bi-clock"></i> Book Time
                        </button>
                    </div>
                </div>
                <div class="table-responsive">
                    <table class="table table-hover align-middle mb-0">
                        <thead>
                            <tr>
                                <th>Project</th>
                                <th>Activity</th>
                                <th>Task</th>
                                <th class="text-end">Hours</th>
                                <th>Status</th>
                            </tr>
                        </thead>
                        <tbody>
                            <?php foreach ($tasks as $task): ?>
                                <tr>
                                    <td><?php echo htmlspecialchars($task['project_name']); ?></td>
                                    <td><?php echo htmlspecialchars($task['activity_name']); ?></td>
                                    <td><?php echo htmlspecialchars($task['task_hierarchy']); ?></td>
                                    <td class="text-end"><?php echo htmlspecialchars($task['booked_hours']); ?></td>
                                    <td><?php echo StatusBadge::html($task['status']); ?></td>
                                </tr>
                            <?php endforeach; ?>
                        </tbody>
                    </table>
                </div>
            </div>
        <?php endif; ?>
    </main>

    <?php if (!empty($tasks)): ?>
    <!-- Book Time Modal -->
    <div class="modal fade" id="bookTimeModal" tabindex="-1" aria-labelledby="bookTimeModalLabel" aria-hidden="true">
        <div class="modal-dialog">
            <div class="modal-content">
                <form method="POST">
                    <?php echo Csrf::field(); ?>
                    <div class="modal-header">
                        <h5 class="modal-title" id="bookTimeModalLabel">Book Time</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <div class="mb-3">
                            <label class="form-label" for="assignmentid">Task</label>
                            <select class="form-select" id="assignmentid" name="assignmentid" required>
                                <option value="">Select a task...</option>
                                <?php foreach ($tasks as $task): ?>
                                    <option value="<?php echo $task['assignmentid']; ?>">
                                        <?php echo htmlspecialchars($task['project_name'] . ' - ' . $task['activity_name'] . ' - ' . $task['task_hierarchy']); ?>
                                        (<?php echo $task['booked_hours']; ?> hours booked)
                                    </option>
                                <?php endforeach; ?>
                            </select>
                        </div>

                        <div class="row g-3">
                            <div class="col-sm-6">
                                <label class="form-label" for="hours">Hours</label>
                                <input type="number" class="form-control" id="hours" name="hours"
                                       min="0.5" step="0.5" placeholder="e.g. 2.5" required>
                            </div>
                            <div class="col-sm-6">
                                <label class="form-label" for="bookeddate">Date</label>
                                <input type="date" class="form-control" id="bookeddate" name="bookeddate"
                                       value="<?php echo date('Y-m-d'); ?>" required>
                            </div>
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <button type="submit" class="btn btn-primary">Book Time</button>
                    </div>
                </form>
            </div>
        </div>
    </div>
    <?php endif; ?>

    <?php include 'includes/footer.php'; ?>
</body>
</html>
