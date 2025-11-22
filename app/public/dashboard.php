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
    $data = $activityService->getUserActivities($eid);
?>
<!DOCTYPE html>
<html lang ="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
	<title>OsoEM - Dashboard</title>
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.4.1/css/bootstrap.min.css">
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.7.1/jquery.min.js"></script>
    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.4.1/js/bootstrap.min.js"></script>
</head>
<body>
    <?php include 'includes/header.php'; ?>
    <div class="container" >
    <div class="row">
        <div class="col-md-8">
            <h2>My Activities</h2>
        </div>
        <div class="col-md-4 text-right">
            <a href="book_time.php" class="btn btn-primary" style="margin-top: 15px;">
                <span class="glyphicon glyphicon-time"></span> Book Time
            </a>
        </div>
    </div>
    <table class="table table-hover">
    <thead>
    <tr>
    <th>Activity</th>
    <th>Project</th>
    <th>Category</th>
    <th>Activity Manager</th>
    <th>Task Count</th>
    <th>Status</th>
    <th></th>
    </tr>
    </thead>
    <tbody>
    <?php
    foreach ($data as $activity){
        echo "<tr>";
        echo "<td>".$activity["activityname"]."</td>";
        echo "<td>".$activity["projectname"]."</td>";
        echo "<td>".$activity["categoryname"]." / ".$activity["subcategoryname"]."</td>";
        echo "<td>".$activity["activity_manager"]."</td>";
        echo "<td>".$activity["task_count"]."</td>";
        
        // Determine status based on dates
        $status = "Not Started";
        if ($activity["actualstart"]) {
            $status = $activity["actualfinish"] ? "Completed" : "In Progress";
        } else if ($activity["plannedstart"] && strtotime($activity["plannedstart"]) <= time()) {
            $status = "Overdue";
        }
        echo "<td><span class='label label-";
        switch($status) {
            case "Completed": echo "success'>Completed"; break;
            case "In Progress": echo "info'>In Progress"; break;
            case "Overdue": echo "danger'>Overdue"; break;
            default: echo "default'>Not Started";
        }
        echo "</span></td>";
        
        echo '<td><form action="activity.php" method="post">';
        echo '<input type="hidden" id="activityid" name="activityid" value="'.$activity["activityid"].'">';
        echo '<button type="submit" class="btn btn-default">Details</button>';
        echo '</form></td>';
        echo "</tr>";
    }
    ?>
    </tbody>
    </table>
    </div>
    
</body>
</html>

