<?php
    require_once '../src/bootstrap.php';

    $activityService = new ActivityService();
    $artefactService = new ArtefactService();
    $projectService = new ProjectService();

    $projectid = (int)($_GET['projectid'] ?? 0);
    $artefacttypeid = (int)($_GET['artefacttypeid'] ?? 0);

    $project = null;
    foreach ($activityService->getUserProjects($eid) as $userProject) {
        if ((int)$userProject['projectid'] === $projectid) {
            $project = $userProject;
        }
    }

    if (!$project) {
        http_response_code(403);
        die('Access denied: you are not a member of this project.');
    }

    $artefactTypes = $artefactService->getArtefactTypes($projectid);
    $typeIds = array_map('intval', array_column($artefactTypes, 'artefacttypeid'));
    if (!in_array($artefacttypeid, $typeIds, true)) {
        $artefacttypeid = 0;
    }
    // Name of the active type filter, shown on the Filter button
    $selectedTypeName = null;
    foreach ($artefactTypes as $type) {
        if ((int)$type['artefacttypeid'] === $artefacttypeid) {
            $selectedTypeName = $type['artefactname'];
        }
    }

    $isProjectManager = $projectService->isProjectManager($projectid, $eid);
    // Leads and managers can act on several artefacts at once, so they get a selection column
    $canSelectArtefacts = $projectService->isProjectLeadOrManager($projectid, $eid);
    $message = '';
    $messageType = '';
    // Set after an artefact is deleted from artefact.php, which redirects back here
    if (isset($_GET['deleted'])) {
        $message = 'Artefact deleted.';
        $messageType = 'success';
    }

    $mutatingActions = ['add_artefact', 'delegate_artefacts'];
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && array_intersect(array_keys($_POST), $mutatingActions) && !Csrf::validate($_POST['csrf_token'] ?? '')) {
        http_response_code(403);
        die('Invalid or expired form submission (CSRF token mismatch). Please refresh the page and try again.');
    }

    // Handle new artefact (project managers only). Linking it to an activity is optional.
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['add_artefact'])) {
        $newartefacttypeid = (int)($_POST['new_artefacttypeid'] ?? 0);
        $artefacttitle = trim($_POST['artefacttitle'] ?? '');
        $artefactdescription = trim($_POST['new_artefactdescription'] ?? '');
        $artefactownerid = (int)($_POST['artefactownerid'] ?? 0);
        $linkactivityid = (int)($_POST['linkactivityid'] ?? 0);
        $ratio = $_POST['ratio'] ?? '';

        $memberIds = array_map('intval', array_column($projectService->getProjectTeamMembers($projectid), 'employeeid'));
        $activityIds = array_map('intval', array_column($activityService->getProjectActivities($projectid), 'activityid'));

        if (!$isProjectManager) {
            $message = 'Only the project manager can add artefacts.';
            $messageType = 'danger';
        } elseif (!in_array($newartefacttypeid, $typeIds, true) || $artefacttitle === '') {
            $message = 'Please select an artefact type and enter a title.';
            $messageType = 'warning';
        } elseif ($artefactownerid && !in_array($artefactownerid, $memberIds, true)) {
            $message = 'The artefact owner must be a member of the project team.';
            $messageType = 'warning';
        } elseif ($linkactivityid && (!in_array($linkactivityid, $activityIds, true) || !is_numeric($ratio) || $ratio <= 0 || $ratio > 1)) {
            $message = 'Please select an activity in this project and provide a ratio between 0 and 1.';
            $messageType = 'warning';
        } elseif ($artefactService->addArtefact($newartefacttypeid, $artefacttitle, $artefactdescription, $artefactownerid ?: null, $linkactivityid ?: null, $linkactivityid ? $ratio : null)) {
            $message = 'Artefact added successfully!';
            $messageType = 'success';
        } else {
            $message = 'Error adding artefact. Please try again.';
            $messageType = 'danger';
        }
    }

    // Handle artefact delegation (project leads and managers only). The new owner must be on the project team.
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['delegate_artefacts'])) {
        $delegateIds = array_values(array_unique(array_map('intval', (array)($_POST['artefactids'] ?? []))));
        $newownerid = (int)($_POST['newownerid'] ?? 0);
        $projectArtefactIds = array_map('intval', array_column($artefactService->getArtefactsForProject($projectid), 'artefactid'));
        $memberIds = array_map('intval', array_column($projectService->getProjectTeamMembers($projectid), 'employeeid'));

        if (!$canSelectArtefacts) {
            $message = 'Only the project lead or project manager can delegate artefacts.';
            $messageType = 'danger';
        } elseif (!$delegateIds || array_diff($delegateIds, $projectArtefactIds)) {
            $message = 'Invalid artefact selection.';
            $messageType = 'warning';
        } elseif (!in_array($newownerid, $memberIds, true)) {
            $message = 'Please select a member of this project team to delegate to.';
            $messageType = 'warning';
        } else {
            $failed = 0;
            foreach ($delegateIds as $delegateId) {
                if (!$artefactService->delegateArtefact($delegateId, $newownerid)) {
                    $failed++;
                }
            }
            if ($failed === 0) {
                $message = count($delegateIds) === 1 ? 'Artefact delegated successfully!' : count($delegateIds) . ' artefacts delegated successfully!';
                $messageType = 'success';
            } else {
                $message = "Error delegating $failed of " . count($delegateIds) . ' artefacts. Please try again.';
                $messageType = 'danger';
            }
        }
    }

    if ($canSelectArtefacts) {
        $teamMembers = $projectService->getProjectTeamMembers($projectid);
    }
    if ($isProjectManager) {
        $projectActivities = $activityService->getProjectActivities($projectid);
    }

    $artefactsQuery = http_build_query(['projectid' => $projectid, 'artefacttypeid' => $artefacttypeid]);

    $artefacts = $artefactService->getArtefactsForProject($projectid);
    if ($artefacttypeid) {
        $artefacts = array_values(array_filter(
            $artefacts,
            function ($artefact) use ($artefacttypeid) { return (int)$artefact['artefacttypeid'] === $artefacttypeid; }
        ));
    }

    $pageTitle = 'Artefacts';
    include 'includes/head.php';
?>
<body>
    <?php include 'includes/header.php'; ?>
    <main class="container app-main">
        <div class="page-header">
            <div>
                <nav aria-label="breadcrumb">
                    <ol class="breadcrumb">
                        <li class="breadcrumb-item"><a href="dashboard.php?projectid=<?php echo $projectid; ?>">Dashboard</a></li>
                        <li class="breadcrumb-item active" aria-current="page">Artefacts</li>
                    </ol>
                </nav>
                <h1 class="h3">Artefacts</h1>
                <p class="page-subtitle"><?php echo htmlspecialchars($project['projectno'] . ' - ' . $project['projectname']); ?></p>
            </div>
        </div>

        <?php if ($message): ?>
            <div class="alert alert-<?php echo $messageType; ?> alert-dismissible fade show" role="alert">
                <?php echo htmlspecialchars($message); ?>
                <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Close"></button>
            </div>
        <?php endif; ?>

        <!-- Artefacts -->
        <section class="dashboard-section">
            <div class="d-flex align-items-center justify-content-end gap-2 mb-2">
                <?php if ($isProjectManager): ?>
                    <button type="button" class="btn btn-sm btn-primary" data-bs-toggle="modal" data-bs-target="#addArtefactModal">
                        <i class="bi bi-plus-lg"></i> Add Artefact
                    </button>
                <?php endif; ?>
                <?php if ($canSelectArtefacts): ?>
                    <button type="button" class="btn btn-sm btn-outline-secondary artefact-selection-action" data-bs-toggle="modal" data-bs-target="#delegateArtefactModal" disabled>
                        <i class="bi bi-person-up"></i> Delegate
                    </button>
                <?php endif; ?>
                <button type="button" class="btn btn-sm <?php echo $artefacttypeid ? 'btn-secondary' : 'btn-outline-secondary'; ?>" data-bs-toggle="modal" data-bs-target="#filterArtefactsModal">
                    <i class="bi bi-funnel"></i> Filter<?php if ($selectedTypeName !== null): ?>: <?php echo htmlspecialchars($selectedTypeName); ?><?php endif; ?>
                </button>
            </div>
            <div class="card">
                <?php if (empty($artefacts)): ?>
                    <div class="empty-state">
                        <i class="bi bi-folder2-open"></i>
                        No artefacts found.
                    </div>
                <?php else: ?>
                    <div class="table-responsive">
                        <table class="table table-hover align-middle mb-0">
                            <thead>
                                <tr>
                                    <?php if ($canSelectArtefacts): ?>
                                        <th class="row-select" style="width: 1%;">
                                            <input class="form-check-input" type="checkbox" id="selectAllArtefacts" aria-label="Select all artefacts">
                                        </th>
                                    <?php endif; ?>
                                    <th>Artefact</th>
                                    <th>Type</th>
                                    <th>Owner</th>
                                    <th style="min-width: 10rem;">Progress</th>
                                </tr>
                            </thead>
                            <tbody>
                                <?php foreach ($artefacts as $artefact): ?>
                                    <?php $artefactUrl = 'artefact.php?artefactid=' . urlencode($artefact['artefactid']); ?>
                                    <tr class="clickable-row" data-href="<?php echo $artefactUrl; ?>">
                                        <?php if ($canSelectArtefacts): ?>
                                            <td class="row-select">
                                                <input class="form-check-input artefact-select" type="checkbox"
                                                       value="<?php echo (int)$artefact['artefactid']; ?>"
                                                       data-artefact-title="<?php echo htmlspecialchars($artefact['title'], ENT_QUOTES); ?>"
                                                       aria-label="Select <?php echo htmlspecialchars($artefact['title'], ENT_QUOTES); ?>">
                                            </td>
                                        <?php endif; ?>
                                        <td class="fw-medium">
                                            <a href="<?php echo $artefactUrl; ?>" class="text-reset text-decoration-none"><?php echo htmlspecialchars($artefact['title']); ?></a>
                                        </td>
                                        <td class="text-body-secondary"><?php echo htmlspecialchars($artefact['artefact']); ?></td>
                                        <td><?php echo htmlspecialchars($artefact['artefactowner'] ?? 'Unassigned'); ?></td>
                                        <td>
                                            <div class="d-flex align-items-center gap-2">
                                                <div class="progress flex-grow-1" role="progressbar"
                                                     aria-valuenow="<?php echo (float)$artefact['progress']; ?>" aria-valuemin="0" aria-valuemax="100">
                                                    <div class="progress-bar <?php echo $artefact['progress'] >= 100 ? 'is-complete' : ''; ?>"
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
                <?php endif; ?>
            </div>
        </section>
    </main>

    <!-- Filter Artefacts Modal -->
    <div class="modal fade" id="filterArtefactsModal" tabindex="-1" aria-labelledby="filterArtefactsModalLabel" aria-hidden="true">
        <div class="modal-dialog modal-sm">
            <div class="modal-content">
                <form method="get" action="artefacts.php">
                    <input type="hidden" name="projectid" value="<?php echo $projectid; ?>">
                    <div class="modal-header">
                        <h5 class="modal-title" id="filterArtefactsModalLabel">Filter Artefacts</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <label for="artefacttypeid" class="form-label">Artefact type</label>
                        <select class="form-select" id="artefacttypeid" name="artefacttypeid">
                            <option value="0">All types</option>
                            <?php foreach ($artefactTypes as $type): ?>
                                <option value="<?php echo (int)$type['artefacttypeid']; ?>" <?php echo (int)$type['artefacttypeid'] === $artefacttypeid ? 'selected' : ''; ?>>
                                    <?php echo htmlspecialchars($type['artefactname']); ?>
                                </option>
                            <?php endforeach; ?>
                        </select>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <button type="submit" class="btn btn-primary">Apply</button>
                    </div>
                </form>
            </div>
        </div>
    </div>

    <?php if ($isProjectManager): ?>
    <!-- Add Artefact Modal -->
    <div class="modal fade" id="addArtefactModal" tabindex="-1" aria-labelledby="addArtefactModalLabel" aria-hidden="true">
        <div class="modal-dialog">
            <div class="modal-content">
                <form method="POST" action="artefacts.php?<?php echo htmlspecialchars($artefactsQuery); ?>">
                    <?php echo Csrf::field(); ?>
                    <div class="modal-header">
                        <h5 class="modal-title" id="addArtefactModalLabel">Add Artefact</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <?php if (!empty($artefactTypes)): ?>
                            <div class="mb-3">
                                <label for="new_artefacttypeid" class="form-label">Artefact type</label>
                                <select class="form-select" id="new_artefacttypeid" name="new_artefacttypeid" required>
                                    <option value="">Please select a type...</option>
                                    <?php foreach ($artefactTypes as $type): ?>
                                        <option value="<?php echo (int)$type['artefacttypeid']; ?>"><?php echo htmlspecialchars($type['artefactname']); ?></option>
                                    <?php endforeach; ?>
                                </select>
                            </div>
                            <div class="mb-3">
                                <label for="artefacttitle" class="form-label">Title</label>
                                <input type="text" class="form-control" id="artefacttitle" name="artefacttitle" maxlength="255"
                                       placeholder="Enter artefact title..." required>
                            </div>
                            <div class="mb-3">
                                <label for="new_artefactdescription" class="form-label">Description</label>
                                <textarea class="form-control" id="new_artefactdescription" name="new_artefactdescription" rows="3"></textarea>
                            </div>
                            <div class="mb-3">
                                <label for="artefactownerid" class="form-label">Owner</label>
                                <select class="form-select" id="artefactownerid" name="artefactownerid">
                                    <option value="">Unassigned</option>
                                    <?php foreach ($teamMembers as $member): ?>
                                        <option value="<?php echo (int)$member['employeeid']; ?>"><?php echo htmlspecialchars($member['fullname']); ?></option>
                                    <?php endforeach; ?>
                                </select>
                            </div>
                            <div class="row g-3">
                                <div class="col-sm-7">
                                    <label for="linkactivityid" class="form-label">Activity</label>
                                    <select class="form-select" id="linkactivityid" name="linkactivityid">
                                        <option value="">Not linked</option>
                                        <?php foreach ($projectActivities as $projectActivity): ?>
                                            <option value="<?php echo (int)$projectActivity['activityid']; ?>"><?php echo htmlspecialchars($projectActivity['activityname']); ?></option>
                                        <?php endforeach; ?>
                                    </select>
                                </div>
                                <div class="col-sm-5">
                                    <label for="ratio" class="form-label">Ratio</label>
                                    <input type="number" class="form-control" id="ratio" name="ratio"
                                           min="0.01" max="1" step="0.01" value="1" disabled>
                                </div>
                            </div>
                            <div class="form-text">Optionally link the artefact to an activity. Ratio is the share of the artefact contributed by that activity (0–1).</div>
                        <?php else: ?>
                            <div class="alert alert-info mb-0">
                                No artefact types exist for this project yet. Add an artefact type first.
                            </div>
                        <?php endif; ?>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <?php if (!empty($artefactTypes)): ?>
                            <button type="submit" name="add_artefact" class="btn btn-primary">Add Artefact</button>
                        <?php endif; ?>
                    </div>
                </form>
            </div>
        </div>
    </div>
    <?php endif; ?>

    <?php if ($canSelectArtefacts): ?>
    <!-- Delegate Artefact Modal -->
    <div class="modal fade" id="delegateArtefactModal" tabindex="-1" aria-labelledby="delegateArtefactModalLabel" aria-hidden="true">
        <div class="modal-dialog">
            <div class="modal-content">
                <form method="POST" action="artefacts.php?<?php echo htmlspecialchars($artefactsQuery); ?>">
                    <?php echo Csrf::field(); ?>
                    <div class="selected-artefact-inputs"></div>
                    <div class="modal-header">
                        <h5 class="modal-title" id="delegateArtefactModalLabel">Delegate Artefacts</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <p class="mb-3"><span class="text-body-secondary">Artefacts:</span> <strong class="selected-artefact-titles"></strong></p>
                        <label for="newownerid" class="form-label">Delegate to</label>
                        <select class="form-select" id="newownerid" name="newownerid" required>
                            <option value="">Please select a user...</option>
                            <?php foreach ($teamMembers as $member): ?>
                                <option value="<?php echo (int)$member['employeeid']; ?>"><?php echo htmlspecialchars($member['fullname']); ?></option>
                            <?php endforeach; ?>
                        </select>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <button type="submit" name="delegate_artefacts" class="btn btn-primary">Delegate</button>
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
            if (!e.target.closest('a, button, .row-select')) {
                window.location.href = row.dataset.href;
            }
        });
    });

    // Artefact multi-selection; header buttons with .artefact-selection-action are enabled while any are selected
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
    // Restore the selection state after a back/forward navigation keeps checkboxes checked
    updateArtefactSelection();

    // Copies the current selection into a modal's hidden inputs and title list
    function fillSelectedArtefacts(modal) {
        var inputs = modal.querySelector('.selected-artefact-inputs');
        inputs.innerHTML = '';
        selectedArtefacts().forEach(function (cb) {
            var input = document.createElement('input');
            input.type = 'hidden';
            input.name = 'artefactids[]';
            input.value = cb.value;
            inputs.appendChild(input);
        });
        modal.querySelector('.selected-artefact-titles').textContent =
            selectedArtefacts().map(function (cb) { return cb.dataset.artefactTitle; }).join(', ');
    }

    var delegateArtefactModal = document.getElementById('delegateArtefactModal');
    if (delegateArtefactModal) {
        delegateArtefactModal.addEventListener('show.bs.modal', function () { fillSelectedArtefacts(delegateArtefactModal); });
    }
    // The ratio only applies when the new artefact is linked to an activity
    var linkActivitySelect = document.getElementById('linkactivityid');
    if (linkActivitySelect) {
        var toggleRatio = function () {
            var ratio = document.getElementById('ratio');
            ratio.disabled = !linkActivitySelect.value;
            ratio.required = !!linkActivitySelect.value;
        };
        linkActivitySelect.addEventListener('change', toggleRatio);
        toggleRatio();
    }
    </script>
</body>
</html>
