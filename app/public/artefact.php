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

    $artefactService = new ArtefactService();
    
    $data = $artefactService->getArtefactDetails($artefactlinkid);
    $artefactid = $data['artefactid'];
    $artefacttypeid = $data['artefacttypeid'];
    $activityid = $data['activityid'];
    $artefactTitle = $data['artefacttitle'];
    $artefactType = $data['artefactname'];
    $project = $data['projectname'];
    
    $fieldlist = [];
    $dataFields = $artefactService->getArtefactDataFields($artefacttypeid);
    foreach ($dataFields as $dataField) {
        $field = [];
        $field['caption'] = $dataField['fieldtitle'];
        $field['value'] = $artefactService->getArtefactFieldValue($artefactid, $dataField['artefactdatafieldid']);
        $fieldlist[] = $field;
    }

    $linkedActivities = $artefactService->getLinkedActivities($artefactid);
    $activityNames = array_map(function($link) {
        return $link['activityname'];
    }, $linkedActivities);
    $linkedActivitiesString = implode(', ', $activityNames);

    $progressData = $artefactService->getArtefactProgressByActivity($artefactid);

    $milestoneName = $artefactService->getMilestoneDetails($artefactlinkid);
    $milestoneName = $data['milestonename'];
    $currentActivity = $data['activityname'];

    $milestonelist = $artefactService->getMilestoneSteps($artefactlinkid);
?>
<!DOCTYPE html>
<html lang ="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
	<title>OsoEM - Artefact</title>
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.4.1/css/bootstrap.min.css">
    <link rel="stylesheet" href="css/activity.css">
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.7.1/jquery.min.js"></script>
    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.4.1/js/bootstrap.min.js"></script>
</head>
<body>

    <?php include 'includes/header.php'; ?>
    <div class="container">
        <div style="margin-bottom: 20px;">
            <form action="activity.php" method="post" style="display: inline;">
                <input type="hidden" name="activityid" value="<?php echo $activityid; ?>">
                <button type="submit" class="btn btn-default">
                    <span class="glyphicon glyphicon-arrow-left"></span> Back to Activity
                </button>
            </form>
        </div>
        <div class="row">
            <div class="col-sm-12">
                <div>
                    <h2><small>ARTEFACT</small></h2>
                    <h2><?php echo htmlspecialchars($data['artefacttitle']); ?></h2>
                </div>
            </div>
        </div>

        <div class="row">
            <div class="col-sm-12">
                <!-- Artefact Details -->
                <p>Type: <?php echo htmlspecialchars($artefactType); ?> </p>
                <p>Owner: <?php echo htmlspecialchars($data['artefactowner'] ?? 'Unassigned'); ?> </p>
                <p>Linked Activities: <?php echo htmlspecialchars($linkedActivitiesString); ?> </p>
                <p>Project: <?php echo htmlspecialchars($project); ?> </p>
            </div>
            <div class="col-sm-12">
                <form>
                    <?php foreach($fieldlist as $field): ?>
                        <?php 
                            $fieldCaption = htmlspecialchars($field['caption']);
                            $fieldValue = htmlspecialchars($field['value']);
                        ?>
                        <div class="form-group">
                            <label for="<?php echo $fieldCaption; ?>"><?php echo $fieldCaption; ?></label>
                            <input type="text" class="form-control" id="<?php echo $fieldCaption;?>" value="<?php echo $fieldValue;?>"> 
                        </div>
                    <?php endforeach; ?>
                </form>
            </div>

            <!-- Progress Visualization -->
            <div class="col-sm-12">
                <h3>Progress</h3>
                <div class="well">
                    <?php
                    $progressChart = new ArtefactProgressChart($progressData);
                    $progressChart->draw();
                    ?>
                </div>
            </div>
        </div> <!-- row -->
        <div class="row">
            <div class="col-sm-12">
                <div>
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

    <script>
    function navigateToMilestones(artefactlinkid) {
        // Create a form and submit it to milestones.php
        var form = document.createElement('form');
        form.method = 'POST';
        form.action = 'milestones.php';

        var input = document.createElement('input');
        input.type = 'hidden';
        input.name = 'artefactlinkid';
        input.value = artefactlinkid;

        form.appendChild(input);
        document.body.appendChild(form);
        form.submit();
    }
    </script>
</body>
</html>
