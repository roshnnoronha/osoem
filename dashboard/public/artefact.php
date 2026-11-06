<?php
    require_once '../src/bootstrap.php';

    // Opened either from an activity (artefactlinkid, gives activity context) or from the artefacts list (artefactid)
    $artefactlinkid = (int)($_GET['artefactlinkid'] ?? 0);
    $artefactid = (int)($_GET['artefactid'] ?? 0);

    $artefactService = new ArtefactService();
    $activityService = new ActivityService();

    $message = '';
    $messageType = '';

    function loadArtefact($artefactService, $artefactlinkid, $artefactid) {
        if ($artefactlinkid) {
            return $artefactService->getArtefactDetails($artefactlinkid);
        }
        return $artefactid ? $artefactService->getArtefactDetailsById($artefactid) : null;
    }

    $data = loadArtefact($artefactService, $artefactlinkid, $artefactid);

    if (!$data) {
        http_response_code(404);
        die('Artefact not found.');
    }

    $artefactid = $data['artefactid'];
    $artefacttypeid = $data['artefacttypeid'];
    $activityid = $data['activityid'] ?? null;
    $artefactType = $data['artefactname'];
    $project = $data['projectname'];

    $isProjectMember = false;
    foreach ($activityService->getUserProjects($eid) as $userProject) {
        if ((int)$userProject['projectid'] === (int)$data['projectid']) {
            $isProjectMember = true;
        }
    }

    if (!$isProjectMember) {
        http_response_code(403);
        die('Access denied: you are not a member of this artefact\'s project.');
    }

    $linkedActivities = $artefactService->getArtefactActivityLinks($artefactid);

    $projectService = new ProjectService();
    $isProjectLeadOrManager = $projectService->isProjectLeadOrManager($data['projectid'], $eid);
    // The artefact owner, project Leads and the project Manager can edit the artefact and its links;
    // only Leads and the Manager can delegate it
    $canEditArtefact = $data['artefactownerid'] == $eid || $isProjectLeadOrManager;
    $canDelegateArtefact = $isProjectLeadOrManager;
    // Only the project Manager can delete the artefact
    $canDeleteArtefact = $projectService->isProjectManager($data['projectid'], $eid);

    // Handle artefact deletion. On success the artefact no longer exists, so return to the project's artefacts list.
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['delete_artefact'])) {
        if (!Csrf::validate($_POST['csrf_token'] ?? '')) {
            http_response_code(403);
            die('Invalid or expired form submission (CSRF token mismatch). Please refresh the page and try again.');
        }

        if (!$canDeleteArtefact) {
            $message = 'Only the project manager can delete this artefact.';
            $messageType = 'danger';
        } elseif ($artefactService->deleteArtefacts([(int)$artefactid])) {
            header('Location: artefacts.php?projectid=' . urlencode($data['projectid']) . '&deleted=1');
            exit;
        } else {
            $message = 'Error deleting artefact. Please try again.';
            $messageType = 'danger';
        }
    }

    // Handle artefact name/description edit
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['edit_artefact'])) {
        if (!Csrf::validate($_POST['csrf_token'] ?? '')) {
            http_response_code(403);
            die('Invalid or expired form submission (CSRF token mismatch). Please refresh the page and try again.');
        }

        $artefacttitle = trim($_POST['edit_artefacttitle'] ?? '');
        $artefactdescription = trim($_POST['edit_artefactdescription'] ?? '');
        $artefactdescription = $artefactdescription !== '' ? $artefactdescription : null;

        if (!$canEditArtefact) {
            $message = 'Only the artefact owner, a project lead or the project manager can edit this artefact.';
            $messageType = 'danger';
        } elseif ($artefacttitle === '') {
            $message = 'Please enter an artefact name.';
            $messageType = 'warning';
        } elseif (mb_strlen($artefacttitle) > 255 || ($artefactdescription !== null && mb_strlen($artefactdescription) > 1000)) {
            $message = 'Artefact name must be at most 255 characters and description at most 1000 characters.';
            $messageType = 'warning';
        } elseif ($artefactService->updateArtefact($artefactid, $artefacttitle, $artefactdescription)) {
            $message = 'Artefact updated successfully!';
            $messageType = 'success';
            $data = loadArtefact($artefactService, $artefactlinkid, $artefactid);
        } else {
            $message = 'Error updating artefact. Please try again.';
            $messageType = 'danger';
        }
    }

    // Handle link ratio edit
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['edit_link_ratios'])) {
        if (!Csrf::validate($_POST['csrf_token'] ?? '')) {
            http_response_code(403);
            die('Invalid or expired form submission (CSRF token mismatch). Please refresh the page and try again.');
        }

        $postedRatios = $_POST['ratios'] ?? [];
        $ratios = [];
        $validRatios = is_array($postedRatios);
        if ($validRatios) {
            foreach ($postedRatios as $linkid => $ratio) {
                if (!is_numeric($ratio) || $ratio <= 0 || $ratio > 1) {
                    $validRatios = false;
                    break;
                }
                $ratios[(int)$linkid] = round((float)$ratio, 2);
            }
        }

        if (!$canEditArtefact) {
            $message = 'Only the artefact owner, a project lead or the project manager can edit the link ratios.';
            $messageType = 'danger';
        } elseif (!$validRatios || empty($ratios)) {
            $message = 'Please provide a ratio between 0 and 1 for every linked activity.';
            $messageType = 'warning';
        } else {
            $result = $artefactService->updateLinkRatios($artefactid, $ratios);
            $message = $result['message'];
            $messageType = $result['success'] ? 'success' : 'danger';
            if ($result['success']) {
                $linkedActivities = $artefactService->getArtefactActivityLinks($artefactid);
            }
        }
    }

    // Handle linking a project activity to this artefact
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['link_activity'])) {
        if (!Csrf::validate($_POST['csrf_token'] ?? '')) {
            http_response_code(403);
            die('Invalid or expired form submission (CSRF token mismatch). Please refresh the page and try again.');
        }

        $linkactivityid = $_POST['link_activityid'] ?? '';
        $ratio = $_POST['link_ratio'] ?? '';
        $projectActivityIds = array_column($activityService->getProjectActivities($data['projectid']), 'activityid');

        if (!$canEditArtefact) {
            $message = 'Only the artefact owner, a project lead or the project manager can link activities.';
            $messageType = 'danger';
        } elseif (!in_array($linkactivityid, $projectActivityIds) || !is_numeric($ratio) || $ratio <= 0 || $ratio > 1) {
            $message = 'Please select an activity in this project and provide a ratio between 0 and 1.';
            $messageType = 'warning';
        } else {
            $result = $artefactService->linkArtefactToActivity($artefactid, (int)$linkactivityid, round((float)$ratio, 2));
            $message = $result['message'];
            $messageType = $result['success'] ? 'success' : 'danger';
            if ($result['success']) {
                $message = 'Activity linked successfully!';
                $linkedActivities = $artefactService->getArtefactActivityLinks($artefactid);
            }
        }
    }

    // Handle removing an activity link from this artefact
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['unlink_activity'])) {
        if (!Csrf::validate($_POST['csrf_token'] ?? '')) {
            http_response_code(403);
            die('Invalid or expired form submission (CSRF token mismatch). Please refresh the page and try again.');
        }

        $unlinkid = (int)($_POST['unlink_artefactlinkid'] ?? 0);
        $artefactLinkIds = array_map('intval', array_column($linkedActivities, 'artefactlinkid'));

        if (!$canEditArtefact) {
            $message = 'Only the artefact owner, a project lead or the project manager can remove activity links.';
            $messageType = 'danger';
        } elseif (!in_array($unlinkid, $artefactLinkIds, true)) {
            $message = 'Invalid activity link.';
            $messageType = 'warning';
        } elseif ($artefactService->unlinkArtefactFromActivity($unlinkid)) {
            // The page was opened through the removed link, so its URL no longer resolves; reopen by artefact id
            if ($unlinkid === $artefactlinkid) {
                header('Location: artefact.php?artefactid=' . urlencode($artefactid) . '&unlinked=1');
                exit;
            }
            $message = 'Activity link removed.';
            $messageType = 'success';
            $linkedActivities = $artefactService->getArtefactActivityLinks($artefactid);
        } else {
            $message = 'Error removing activity link. Please try again.';
            $messageType = 'danger';
        }
    }

    // Set after removing the link this page was opened through, which redirects back here
    if (isset($_GET['unlinked']) && $message === '') {
        $message = 'Activity link removed.';
        $messageType = 'success';
    }

    // Handle artefact delegation
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['delegate_artefact'])) {
        if (!Csrf::validate($_POST['csrf_token'] ?? '')) {
            http_response_code(403);
            die('Invalid or expired form submission (CSRF token mismatch). Please refresh the page and try again.');
        }

        $newownerid = $_POST['newownerid'] ?? '';
        if (!$canDelegateArtefact) {
            $message = 'Only a project lead or the project manager can delegate this artefact.';
            $messageType = 'danger';
        } elseif ($newownerid && !in_array((int)$newownerid, array_map('intval', array_column($projectService->getProjectTeamMembers($data['projectid']), 'employeeid')), true)) {
            $message = 'Please select a member of this project team to delegate to.';
            $messageType = 'warning';
        } elseif ($newownerid) {
            if ($artefactService->delegateArtefact($artefactid, $newownerid)) {
                $message = 'Artefact delegated successfully!';
                $messageType = 'success';
                $data = loadArtefact($artefactService, $artefactlinkid, $artefactid);
            } else {
                $message = 'Error delegating artefact. Please try again.';
                $messageType = 'danger';
            }
        } else {
            $message = 'Please select a user to delegate to.';
            $messageType = 'warning';
        }
    }

    $fieldlist = [];
    foreach ($artefactService->getArtefactDataFields($artefacttypeid) as $dataField) {
        $fieldid = (int)$dataField['artefactdatafieldid'];
        $fieldlist[] = [
            'caption' => $dataField['fieldtitle'],
            'value' => $artefactService->getArtefactFieldValue($artefactid, $fieldid),
        ];
    }

    $milestoneService = new MilestoneService();
    $milestoneSteps = [];
    foreach ($linkedActivities as $link) {
        $milestoneSteps[(int)$link['artefactlinkid']] = $milestoneService->getMilestoneSteps($link['artefactlinkid']);
    }

    // Project activities not yet linked, and the ratio still unallocated across the artefact's links
    $linkableActivities = [];
    $remainingRatio = 0;
    if ($canEditArtefact) {
        $linkedActivityIds = array_column($linkedActivities, 'activityid');
        foreach ($activityService->getProjectActivities($data['projectid']) as $projectActivity) {
            if (!in_array($projectActivity['activityid'], $linkedActivityIds)) {
                $linkableActivities[] = $projectActivity;
            }
        }
        $remainingRatio = round(1 - array_sum(array_column($linkedActivities, 'ratio')), 2);
    }

    // Artefacts can only be delegated to members of the project team
    $projectTeamMembers = $projectService->getProjectTeamMembers($data['projectid']);

    $activityName = '';
    foreach ($linkedActivities as $link) {
        if ($link['activityid'] == $activityid) {
            $activityName = $link['activityname'];
        }
    }

    $pageTitle = $data['artefacttitle'];
    include 'includes/head.php';
?>
<body>
    <?php include 'includes/header.php'; ?>
    <main class="container app-main">
        <div class="page-header">
            <div>
                <nav aria-label="breadcrumb">
                    <ol class="breadcrumb">
                        <li class="breadcrumb-item"><a href="dashboard.php?projectid=<?php echo urlencode($data['projectid']); ?>">Dashboard</a></li>
                        <?php if ($activityid !== null): ?>
                            <li class="breadcrumb-item"><a href="activity.php?activityid=<?php echo urlencode($activityid); ?>"><?php echo htmlspecialchars($activityName); ?></a></li>
                        <?php else: ?>
                            <li class="breadcrumb-item"><a href="artefacts.php?projectid=<?php echo urlencode($data['projectid']); ?>">Artefacts</a></li>
                        <?php endif; ?>
                        <li class="breadcrumb-item active" aria-current="page"><?php echo htmlspecialchars($data['artefacttitle']); ?></li>
                    </ol>
                </nav>
                <div class="section-label"><?php echo htmlspecialchars($artefactType); ?></div>
                <h1 class="h3"><?php echo htmlspecialchars($data['artefacttitle']); ?></h1>
                <p class="page-subtitle"><?php echo htmlspecialchars($project); ?></p>
            </div>
        </div>

        <?php if ($message): ?>
            <div class="alert alert-<?php echo $messageType; ?> alert-dismissible fade show" role="alert">
                <?php echo htmlspecialchars($message); ?>
                <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Close"></button>
            </div>
        <?php endif; ?>

        <div class="row g-4">
            <div class="col-12">
                <div class="card">
                    <div class="card-header">
                        <h2 class="card-title">Overview</h2>
                        <div class="card-actions">
                            <?php if ($canEditArtefact): ?>
                                <button type="button" class="btn btn-sm btn-outline-secondary" data-bs-toggle="modal" data-bs-target="#editArtefactModal">
                                    <i class="bi bi-pencil"></i> Edit Artefact
                                </button>
                            <?php endif; ?>
                            <?php if ($canDelegateArtefact): ?>
                                <button type="button" class="btn btn-sm btn-outline-secondary" data-bs-toggle="modal" data-bs-target="#delegateArtefactModal">
                                    <i class="bi bi-person-up"></i> Delegate Artefact
                                </button>
                            <?php endif; ?>
                            <?php if ($canDeleteArtefact): ?>
                                <button type="button" class="btn btn-sm btn-outline-danger" data-bs-toggle="modal" data-bs-target="#deleteArtefactModal">
                                    <i class="bi bi-trash"></i> Delete Artefact
                                </button>
                            <?php endif; ?>
                        </div>
                    </div>
                    <div class="card-body">
                        <?php if (!empty($data['artefactdescription'])): ?>
                            <p><?php echo htmlspecialchars($data['artefactdescription']); ?></p>
                        <?php endif; ?>
                        <dl class="row detail-list">
                            <dt class="col-sm-4 col-lg-3">Type</dt>
                            <dd class="col-sm-8 col-lg-9"><?php echo htmlspecialchars($artefactType); ?></dd>
                            <dt class="col-sm-4 col-lg-3">Owner</dt>
                            <dd class="col-sm-8 col-lg-9"><?php echo htmlspecialchars($data['artefactowner'] ?? 'Unassigned'); ?></dd>
                        </dl>

                        <div class="section-label border-top pt-3 mb-2">Milestone progress</div>
                        <?php echo (new MilestoneProgressChart($linkedActivities, $milestoneSteps))->draw(); ?>
                    </div>
                </div>
            </div>

            <div class="col-12">
                <div class="card">
                    <div class="card-header">
                        <h2 class="card-title">Data</h2>
                        <div class="card-actions">
                            <a href="artefact_data.php?artefactid=<?php echo urlencode($data['artefactid']); ?>" class="btn btn-sm btn-outline-secondary">
                                <i class="bi bi-pencil"></i> Edit Data
                            </a>
                        </div>
                    </div>
                    <div class="card-body">
                        <?php if (empty($fieldlist)): ?>
                            <div class="empty-state">
                                <i class="bi bi-inbox"></i>
                                This artefact type has no data fields.
                            </div>
                        <?php else: ?>
                            <dl class="row detail-list">
                                <?php foreach ($fieldlist as $field): ?>
                                    <dt class="col-sm-4 col-lg-3"><?php echo htmlspecialchars($field['caption']); ?></dt>
                                    <dd class="col-sm-8 col-lg-9"><?php echo $field['value'] !== '' ? htmlspecialchars($field['value']) : '<span class="text-muted">&mdash;</span>'; ?></dd>
                                <?php endforeach; ?>
                            </dl>
                        <?php endif; ?>
                    </div>
                </div>
            </div>

            <div class="col-12">
                <div class="card">
                    <div class="card-header">
                        <h2 class="card-title">Linked activities</h2>
                        <?php if ($canEditArtefact): ?>
                            <div class="card-actions">
                                <button type="button" class="btn btn-sm btn-outline-secondary" data-bs-toggle="modal" data-bs-target="#linkActivityModal">
                                    <i class="bi bi-link-45deg"></i> Link Activity
                                </button>
                                <?php if (!empty($linkedActivities)): ?>
                                    <button type="button" class="btn btn-sm btn-outline-secondary" data-bs-toggle="modal" data-bs-target="#editLinkRatiosModal">
                                        <i class="bi bi-pencil"></i> Edit Links
                                    </button>
                                    <button type="button" class="btn btn-sm btn-outline-danger" data-bs-toggle="modal" data-bs-target="#unlinkActivityModal">
                                        <i class="bi bi-x-lg"></i> Remove Link
                                    </button>
                                <?php endif; ?>
                            </div>
                        <?php endif; ?>
                    </div>
                    <?php if (empty($linkedActivities)): ?>
                        <div class="empty-state">
                            <i class="bi bi-link-45deg"></i>
                            No linked activities.
                        </div>
                    <?php else: ?>
                        <div class="table-responsive">
                            <table class="table table-hover align-middle mb-0">
                                <thead>
                                    <tr>
                                        <th>Activity</th>
                                        <th>Link Ratio</th>
                                        <th style="min-width: 10rem;">Progress</th>
                                    </tr>
                                </thead>
                                <tbody>
                                    <?php foreach ($linkedActivities as $link): ?>
                                        <?php
                                            $linkid = (int)$link['artefactlinkid'];
                                            $steps = $milestoneSteps[$linkid];
                                        ?>
                                        <tr class="clickable-row<?php echo $linkid === $artefactlinkid ? ' table-active' : ''; ?>" data-href="activity.php?activityid=<?php echo urlencode($link['activityid']); ?>">
                                            <td>
                                                <a href="activity.php?activityid=<?php echo urlencode($link['activityid']); ?>">
                                                    <?php echo htmlspecialchars($link['activityname']); ?>
                                                </a>
                                            </td>
                                            <td><?php echo number_format((float)$link['ratio'], 2); ?></td>
                                            <td>
                                                <div class="d-flex align-items-center gap-2">
                                                    <div class="progress flex-grow-1" role="progressbar"
                                                         aria-valuenow="<?php echo (float)$link['progress']; ?>" aria-valuemin="0" aria-valuemax="100">
                                                        <div class="progress-bar <?php echo $link['progress'] >= 100 ? 'is-complete' : ''; ?>"
                                                             style="width: <?php echo (float)$link['progress']; ?>%"></div>
                                                    </div>
                                                    <span class="small text-body-secondary" style="min-width: 2.5rem;"><?php echo number_format($link['progress'], 0); ?>%</span>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <td colspan="3" class="bg-body-tertiary">
                                                <?php if (empty($steps)): ?>
                                                    <div class="small text-body-secondary">
                                                        <i class="bi bi-flag"></i> No milestone steps are linked to this activity.
                                                    </div>
                                                <?php else: ?>
                                                    <table class="table table-sm align-middle mb-0">
                                                        <thead>
                                                            <tr>
                                                                <th>Milestone Step</th>
                                                                <th>Planned</th>
                                                                <th>Forecast</th>
                                                                <th>Achieved</th>
                                                                <th>Completed By</th>
                                                            </tr>
                                                        </thead>
                                                        <tbody>
                                                            <?php foreach ($steps as $step): ?>
                                                                <tr>
                                                                    <td class="fw-medium"><?php echo htmlspecialchars($step['milestonestepname']); ?></td>
                                                                    <td><?php echo htmlspecialchars($step['planned_date'] ?? '-'); ?></td>
                                                                    <td><?php echo htmlspecialchars($step['forecast_date'] ?? '-'); ?></td>
                                                                    <td><?php echo htmlspecialchars($step['completiondate'] ?? '-'); ?></td>
                                                                    <td><?php echo htmlspecialchars($step['completed_by']); ?></td>
                                                                </tr>
                                                            <?php endforeach; ?>
                                                        </tbody>
                                                    </table>
                                                <?php endif; ?>
                                            </td>
                                        </tr>
                                    <?php endforeach; ?>
                                </tbody>
                            </table>
                        </div>
                    <?php endif; ?>
                </div>
            </div>
        </div>
    </main>

    <?php if ($canEditArtefact): ?>
    <!-- Edit Artefact Modal -->
    <div class="modal fade" id="editArtefactModal" tabindex="-1" aria-labelledby="editArtefactModalLabel" aria-hidden="true">
        <div class="modal-dialog">
            <div class="modal-content">
                <form method="POST">
                    <?php echo Csrf::field(); ?>
                    <div class="modal-header">
                        <h5 class="modal-title" id="editArtefactModalLabel">Edit Artefact</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <div class="mb-3">
                            <label for="edit_artefacttitle" class="form-label">Artefact name</label>
                            <input type="text" class="form-control" id="edit_artefacttitle" name="edit_artefacttitle" maxlength="255"
                                   value="<?php echo htmlspecialchars($data['artefacttitle']); ?>" required>
                        </div>
                        <div>
                            <label for="edit_artefactdescription" class="form-label">Description</label>
                            <textarea class="form-control" id="edit_artefactdescription" name="edit_artefactdescription" rows="3" maxlength="1000"><?php echo htmlspecialchars($data['artefactdescription'] ?? ''); ?></textarea>
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <button type="submit" name="edit_artefact" class="btn btn-primary">Save</button>
                    </div>
                </form>
            </div>
        </div>
    </div>
    <?php endif; ?>

    <?php if ($canEditArtefact): ?>
    <!-- Link Activity Modal -->
    <div class="modal fade" id="linkActivityModal" tabindex="-1" aria-labelledby="linkActivityModalLabel" aria-hidden="true">
        <div class="modal-dialog">
            <div class="modal-content">
                <form method="POST">
                    <?php echo Csrf::field(); ?>
                    <div class="modal-header">
                        <h5 class="modal-title" id="linkActivityModalLabel">Link Activity</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <?php if (empty($linkableActivities)): ?>
                            <div class="alert alert-info mb-0">
                                No activities available to link. Every activity in this project is already linked to this artefact.
                            </div>
                        <?php elseif ($remainingRatio <= 0): ?>
                            <div class="alert alert-info mb-0">
                                This artefact's ratio is fully allocated. Reduce the ratio of an existing link before linking another activity.
                            </div>
                        <?php else: ?>
                            <div class="mb-3">
                                <label for="linkActivitySelect" class="form-label">Activity</label>
                                <select class="form-select" id="linkActivitySelect" name="link_activityid" required>
                                    <option value="">Please select an activity...</option>
                                    <?php foreach ($linkableActivities as $linkable): ?>
                                        <option value="<?php echo (int)$linkable['activityid']; ?>">
                                            <?php echo htmlspecialchars($linkable['activityname']); ?>
                                        </option>
                                    <?php endforeach; ?>
                                </select>
                            </div>
                            <div>
                                <label for="linkRatio" class="form-label">Ratio</label>
                                <input type="number" class="form-control" id="linkRatio" name="link_ratio"
                                       min="0.01" max="<?php echo number_format($remainingRatio, 2, '.', ''); ?>" step="0.01"
                                       value="<?php echo number_format($remainingRatio, 2, '.', ''); ?>" required>
                                <div class="form-text">Share of the artefact's work done in this activity (unallocated: <?php echo number_format($remainingRatio, 2); ?>).</div>
                            </div>
                        <?php endif; ?>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <?php if (!empty($linkableActivities) && $remainingRatio > 0): ?>
                            <button type="submit" name="link_activity" class="btn btn-primary">Link</button>
                        <?php endif; ?>
                    </div>
                </form>
            </div>
        </div>
    </div>
    <?php endif; ?>

    <?php if ($canEditArtefact && !empty($linkedActivities)): ?>
    <!-- Edit Link Ratios Modal -->
    <div class="modal fade" id="editLinkRatiosModal" tabindex="-1" aria-labelledby="editLinkRatiosModalLabel" aria-hidden="true">
        <div class="modal-dialog">
            <div class="modal-content">
                <form method="POST">
                    <?php echo Csrf::field(); ?>
                    <div class="modal-header">
                        <h5 class="modal-title" id="editLinkRatiosModalLabel">Edit Link Ratios</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <?php foreach ($linkedActivities as $link): ?>
                            <?php $linkid = (int)$link['artefactlinkid']; ?>
                            <div class="row g-2 align-items-center mb-2">
                                <label for="ratio_<?php echo $linkid; ?>" class="col-8 col-form-label"><?php echo htmlspecialchars($link['activityname']); ?></label>
                                <div class="col-4">
                                    <input type="number" class="form-control link-ratio-input" id="ratio_<?php echo $linkid; ?>" name="ratios[<?php echo $linkid; ?>]"
                                           min="0.01" max="1" step="0.01" value="<?php echo number_format((float)$link['ratio'], 2, '.', ''); ?>" required>
                                </div>
                            </div>
                        <?php endforeach; ?>
                        <div class="form-text" id="linkRatioTotal"></div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <button type="submit" name="edit_link_ratios" class="btn btn-primary" id="saveLinkRatios">Save</button>
                    </div>
                </form>
            </div>
        </div>
    </div>
    <?php endif; ?>

    <?php if ($canEditArtefact && !empty($linkedActivities)): ?>
    <!-- Remove Activity Link Modal -->
    <div class="modal fade" id="unlinkActivityModal" tabindex="-1" aria-labelledby="unlinkActivityModalLabel" aria-hidden="true">
        <div class="modal-dialog">
            <div class="modal-content">
                <form method="POST">
                    <?php echo Csrf::field(); ?>
                    <div class="modal-header">
                        <h5 class="modal-title" id="unlinkActivityModalLabel">Remove Activity Link</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <div class="mb-3">
                            <label for="unlinkArtefactLinkId" class="form-label">Activity</label>
                            <select class="form-select" id="unlinkArtefactLinkId" name="unlink_artefactlinkid" required>
                                <option value="">Please select a linked activity...</option>
                                <?php foreach ($linkedActivities as $link): ?>
                                    <option value="<?php echo (int)$link['artefactlinkid']; ?>">
                                        <?php echo htmlspecialchars($link['activityname']); ?> (ratio <?php echo number_format((float)$link['ratio'], 2); ?>)
                                    </option>
                                <?php endforeach; ?>
                            </select>
                        </div>
                        <div class="alert alert-danger mb-0">
                            <i class="bi bi-exclamation-triangle me-1"></i>
                            The link will be removed together with any milestone progress recorded against it. This cannot be undone.
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <button type="submit" name="unlink_activity" class="btn btn-danger">Remove Link</button>
                    </div>
                </form>
            </div>
        </div>
    </div>
    <?php endif; ?>

    <?php if ($canDelegateArtefact): ?>
    <!-- Delegate Artefact Modal -->
    <div class="modal fade" id="delegateArtefactModal" tabindex="-1" aria-labelledby="delegateArtefactModalLabel" aria-hidden="true">
        <div class="modal-dialog">
            <div class="modal-content">
                <form method="POST">
                    <?php echo Csrf::field(); ?>
                    <div class="modal-header">
                        <h5 class="modal-title" id="delegateArtefactModalLabel">Delegate Artefact</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <label for="newownerid" class="form-label">Delegate to</label>
                        <select class="form-select" id="newownerid" name="newownerid" required>
                            <option value="">Please select a user...</option>
                            <?php foreach ($projectTeamMembers as $employee): ?>
                                <option value="<?php echo $employee['employeeid']; ?>">
                                    <?php echo htmlspecialchars($employee['fullname']); ?>
                                </option>
                            <?php endforeach; ?>
                        </select>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <button type="submit" name="delegate_artefact" class="btn btn-primary">Delegate</button>
                    </div>
                </form>
            </div>
        </div>
    </div>
    <?php endif; ?>

    <?php if ($canDeleteArtefact): ?>
    <!-- Delete Artefact Modal -->
    <div class="modal fade" id="deleteArtefactModal" tabindex="-1" aria-labelledby="deleteArtefactModalLabel" aria-hidden="true">
        <div class="modal-dialog">
            <div class="modal-content">
                <form method="POST">
                    <?php echo Csrf::field(); ?>
                    <div class="modal-header">
                        <h5 class="modal-title" id="deleteArtefactModalLabel">Delete Artefact</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <p class="mb-3"><span class="text-body-secondary">Artefact:</span> <strong><?php echo htmlspecialchars($data['artefacttitle']); ?></strong></p>
                        <div class="alert alert-danger mb-0">
                            <i class="bi bi-exclamation-triangle me-1"></i>
                            The artefact will be permanently deleted, together with its data, its links to activities and any milestone progress recorded against it. This cannot be undone.
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <button type="submit" name="delete_artefact" class="btn btn-danger">Delete</button>
                    </div>
                </form>
            </div>
        </div>
    </div>
    <?php endif; ?>

    <?php include 'includes/footer.php'; ?>
    <script>
    document.querySelectorAll('.clickable-row[data-href]').forEach(function (row) {
        row.addEventListener('click', function (e) {
            if (!e.target.closest('a, button')) {
                window.location.href = row.dataset.href;
            }
        });
    });

    // Show the running total of the link ratios and block saving when it exceeds 1
    var ratioInputs = document.querySelectorAll('.link-ratio-input');
    function updateRatioTotal() {
        var total = 0;
        ratioInputs.forEach(function (input) {
            total += parseFloat(input.value) || 0;
        });
        total = Math.round(total * 100) / 100;
        var help = document.getElementById('linkRatioTotal');
        help.textContent = 'Total: ' + total.toFixed(2) + ' (must not exceed 1.00)';
        help.classList.toggle('text-danger', total > 1);
        document.getElementById('saveLinkRatios').disabled = total > 1;
    }
    if (ratioInputs.length) {
        ratioInputs.forEach(function (input) {
            input.addEventListener('input', updateRatioTotal);
        });
        updateRatioTotal();
    }
    </script>
</body>
</html>
