<?php
    session_start();
    require_once '../src/autoload.php';

    if (isset($_POST['artefactlinkid']))
        $artefactlinkid = $_POST['artefactlinkid'];

    $authService = new AuthService();
    $eid = $authService->validateSession();

    if (!$eid) {
        header('Location: index.php');
        exit();
    }

    $milestoneService = new MilestoneService();
    $data = $milestoneService->getMilestoneDetails($artefactlinkid);
    $milestoneName = $data['milestonename'];
    $artefactTitle = $data['artefacttitle'];
    $activityName = $data['activityname'];

    $milestonelist = $milestoneService->getMilestoneSteps($artefactlinkid);
?>
<!DOCTYPE html>
<html lang ="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
	<title>OsoEM - Milestones</title>
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.4.1/css/bootstrap.min.css">
    <link rel="stylesheet" href="css/activity.css">
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.7.1/jquery.min.js"></script>
    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.4.1/js/bootstrap.min.js"></script>
</head>
<body>

    <?php include 'includes/header.php'; ?>
    <div class="container">
        <div style="margin-bottom: 20px;">
            <form action="artefact.php" method="post" style="display: inline;">
                <input type="hidden" name="artefactlinkid" value="<?php echo $artefactlinkid; ?>">
                <button type="submit" class="btn btn-default">
                    <span class="glyphicon glyphicon-arrow-left"></span> Back to Artefact
                </button>
            </form>
        </div>
        <div class="row">
            <div class="col-sm-12">
                <div>
                    <h2><small>MILESTONES</small></h2>
                    <h2><?php echo htmlspecialchars($milestoneName); ?></h2>
                </div>
            </div>
        </div>

        <div class="row">
            <div class="col-sm-12">
                <!-- Artefact Details -->
                <p>Artefact: <?php echo htmlspecialchars($artefactTitle ); ?> </p>
                <p>Activity: <?php echo htmlspecialchars($activityName); ?> </p>
            </div>

            <!-- Progress Card -->
            <div class="col-sm-12">
                <h3>Steps</h3>
                <table class="table table-hover">
                    <thead>
                        <tr>
                            <th>Milestone Step</th>
                            <th>Status</th>
                            <th>Completed By</th>
                        </tr>
                    </thead>
                    <tbody>
                        <?php foreach($milestonelist as $milestone): ?>
                            <tr>
                                <td><?php echo htmlspecialchars($milestone['milestonestepname']); ?></td>
                                <td><?php echo htmlspecialchars($milestone['Status']); ?></td>
                                <td><?php echo htmlspecialchars($milestone['Completed_By']); ?></td>
                            </tr>
                        <?php endforeach; ?>
                    </tbody>
                </table>
            </div>
        </div> <!-- row -->
    </div>

</body>
</html>
