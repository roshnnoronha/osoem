<?php
    require_once '../src/bootstrap.php';

    $activityService = new ActivityService();
    $artefactService = new ArtefactService();
    $projectService = new ProjectService();

    // Restrict the selected project to those the user belongs to. Default to the
    // first project where the user has activities, otherwise the first project.
    $projects = $activityService->getUserProjects($eid);
    $projectIds = array_map('intval', array_column($projects, 'projectid'));
    $userActivities = $activityService->getUserActivities($eid);
    $projectid = isset($_GET['projectid']) ? (int)$_GET['projectid'] : 0;
    if (!in_array($projectid, $projectIds, true)) {
        $projectid = $userActivities ? (int)$userActivities[0]['projectid'] : ($projectIds[0] ?? 0);
    }

    $isProjectManager = $projectid && $projectService->isProjectManager($projectid, $eid);
    $message = '';
    $messageType = '';

    // Handle new activity (project managers only)
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['add_activity'])) {
        if (!Csrf::validate($_POST['csrf_token'] ?? '')) {
            http_response_code(403);
            die('Invalid or expired form submission (CSRF token mismatch). Please refresh the page and try again.');
        }

        $activityname = trim($_POST['activityname'] ?? '');
        $activitydescription = trim($_POST['activitydescription'] ?? '');
        $isNewCategory = ($_POST['subcategoryid'] ?? '') === 'new';
        $subcategoryid = (int)($_POST['subcategoryid'] ?? 0);
        $newcategoryname = trim($_POST['newcategoryname'] ?? '');
        $newsubcategoryname = trim($_POST['newsubcategoryname'] ?? '');
        $activitymanagerid = (int)($_POST['activitymanagerid'] ?? 0);
        $plannedstart = !empty($_POST['plannedstart']) ? $_POST['plannedstart'] : null;
        $plannedfinish = !empty($_POST['plannedfinish']) ? $_POST['plannedfinish'] : null;
        $plannedhours = !empty($_POST['plannedhours']) ? $_POST['plannedhours'] : null;

        $subcategoryIds = array_map('intval', array_column($projectService->getSubcategories($projectid), 'subcategoryid'));
        $memberIds = array_map('intval', array_column($projectService->getProjectTeamMembers($projectid), 'employeeid'));

        if (!$isProjectManager) {
            $message = 'Only the project manager can add activities.';
            $messageType = 'danger';
        } elseif ($activityname === '' || !in_array($activitymanagerid, $memberIds, true)) {
            $message = 'Please enter an activity name and activity manager.';
            $messageType = 'warning';
        } elseif ($isNewCategory ? ($newcategoryname === '' || $newsubcategoryname === '') : !in_array($subcategoryid, $subcategoryIds, true)) {
            $message = $isNewCategory ? 'Please enter both a category and a subcategory name.' : 'Please select a category.';
            $messageType = 'warning';
        } elseif ($plannedstart && $plannedfinish && $plannedfinish < $plannedstart) {
            $message = 'Planned finish cannot be before planned start.';
            $messageType = 'warning';
        } else {
            if ($isNewCategory) {
                $subcategoryid = $projectService->findOrCreateSubcategory($projectid, $newcategoryname, $newsubcategoryname);
            }
            $newActivityId = $subcategoryid ? $activityService->addActivity($subcategoryid, $activityname, $activitydescription, $activitymanagerid, $plannedstart, $plannedfinish, $plannedhours) : false;
            if ($newActivityId) {
                header('Location: activity.php?activityid=' . $newActivityId);
                exit();
            }
            $message = 'Error adding activity. Please try again.';
            $messageType = 'danger';
        }
    }

    // Handle new artefact type (project managers only)
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['add_artefact_type'])) {
        if (!Csrf::validate($_POST['csrf_token'] ?? '')) {
            http_response_code(403);
            die('Invalid or expired form submission (CSRF token mismatch). Please refresh the page and try again.');
        }

        $artefactname = trim($_POST['artefactname'] ?? '');
        $artefacttypedescription = trim($_POST['artefacttypedescription'] ?? '');

        // Data fields arrive as parallel arrays (one entry per row); rows with a blank title are ignored
        $optionalInt = function ($value) {
            $value = trim((string)$value);
            return $value === '' ? null : (filter_var($value, FILTER_VALIDATE_INT) === false ? false : (int)$value);
        };
        $postedFields = $_POST['fields'] ?? [];
        $fields = [];
        $fieldError = '';
        foreach ((array)($postedFields['fieldtitle'] ?? []) as $i => $fieldtitle) {
            $fieldtitle = trim((string)$fieldtitle);
            if ($fieldtitle === '') {
                continue;
            }
            $isNumber = ($postedFields['valuetype'][$i] ?? '0') === '1';
            $maximumlength = $optionalInt($postedFields['maximumlength'][$i] ?? '');
            $minimumvalue = $isNumber ? $optionalInt($postedFields['minimumvalue'][$i] ?? '') : null;
            $maximumvalue = $isNumber ? $optionalInt($postedFields['maximumvalue'][$i] ?? '') : null;

            if (mb_strlen($fieldtitle) > 255) {
                $fieldError = "Field title \"$fieldtitle\" is too long.";
            } elseif ($maximumlength === false || $minimumvalue === false || $maximumvalue === false) {
                $fieldError = "Field \"$fieldtitle\" has a limit that is not a whole number.";
            } elseif ($maximumlength !== null && ($maximumlength < 1 || $maximumlength > 2000)) {
                $fieldError = "Field \"$fieldtitle\" maximum length must be between 1 and 2000.";
            } elseif ($minimumvalue !== null && $maximumvalue !== null && $minimumvalue > $maximumvalue) {
                $fieldError = "Field \"$fieldtitle\" minimum value cannot exceed its maximum value.";
            }
            if ($fieldError !== '') {
                break;
            }

            $fields[] = [
                'fieldtitle' => $fieldtitle,
                'valuetype' => $isNumber ? 1 : 0,
                'maximumlength' => $maximumlength,
                'minimumvalue' => $minimumvalue,
                'maximumvalue' => $maximumvalue,
                'editable' => ($postedFields['editable'][$i] ?? '1') === '1',
            ];
        }

        if (!$isProjectManager) {
            $message = 'Only the project manager can add artefact types.';
            $messageType = 'danger';
        } elseif ($artefactname === '') {
            $message = 'Please enter an artefact type name.';
            $messageType = 'warning';
        } elseif ($fieldError !== '') {
            $message = $fieldError;
            $messageType = 'warning';
        } elseif ($artefactService->addArtefactType($projectid, $artefactname, $artefacttypedescription, $fields)) {
            $message = 'Artefact type added successfully!';
            $messageType = 'success';
        } else {
            $message = 'Error adding artefact type. Please try again.';
            $messageType = 'danger';
        }
    }

    if ($isProjectManager) {
        $subcategories = $projectService->getSubcategories($projectid);
        $categories = $projectService->getCategories($projectid);
        $teamMembers = $projectService->getProjectTeamMembers($projectid);
    }

    // Activity filter: "mine" (activity manager or assigned a task) or "all"
    $activityFilter = ($_GET['activities'] ?? '') === 'all' ? 'all' : 'mine';
    $activitiesTitle = $activityFilter === 'all' ? 'All Activities' : 'My Activities';
    $data = $projectid ? $activityService->getProjectActivities($projectid, $activityFilter === 'all' ? null : $eid) : [];
    $artefacts = $projectid ? $artefactService->getArtefactsForProject($projectid) : [];

    // Artefact filter: "mine" (owned by the user) or "all"
    $artefactFilter = ($_GET['artefacts'] ?? '') === 'all' ? 'all' : 'mine';
    $artefactsTitle = $artefactFilter === 'all' ? 'All Artefacts' : 'My Artefacts';
    if ($artefactFilter === 'mine') {
        $artefacts = array_values(array_filter($artefacts, function ($artefact) use ($eid) {
            return (int)$artefact['artefactownerid'] === (int)$eid;
        }));
    }

    // Query string that keeps the selected project and filters across form submissions
    $dashboardQuery = http_build_query(['projectid' => $projectid, 'activities' => $activityFilter, 'artefacts' => $artefactFilter]);

    // Summarise artefacts by type. Cumulative progress is the mean progress of the type's artefacts.
    $artefactTypes = [];
    $projectArtefactTypes = $projectid ? $artefactService->getArtefactTypes($projectid) : [];
    foreach ($projectArtefactTypes as $type) {
        $artefactTypes[(int)$type['artefacttypeid']] = [
            'name' => $type['artefactname'],
            'count' => 0,
            'completed' => 0,
            'progressTotal' => 0,
        ];
    }
    foreach ($artefacts as $artefact) {
        $typeid = (int)$artefact['artefacttypeid'];
        if (!isset($artefactTypes[$typeid])) {
            continue;
        }
        $artefactTypes[$typeid]['count']++;
        $artefactTypes[$typeid]['progressTotal'] += $artefact['progress'];
        if ($artefact['progress'] >= 100) {
            $artefactTypes[$typeid]['completed']++;
        }
    }
    foreach ($artefactTypes as &$type) {
        $type['progress'] = $type['count'] ? $type['progressTotal'] / $type['count'] : 0;
    }
    unset($type);
    // "My Artefacts" only lists the types the user has artefacts in
    if ($artefactFilter === 'mine') {
        $artefactTypes = array_filter($artefactTypes, function ($type) { return $type['count'] > 0; });
    }

    // Determine status based on dates
    $statusCounts = ['In Progress' => 0, 'Overdue' => 0, 'Completed' => 0];
    foreach ($data as &$activity) {
        $status = "Not Started";
        if ($activity["actualstart"]) {
            $status = $activity["actualfinish"] ? "Completed" : "In Progress";
        } else if ($activity["plannedstart"] && strtotime($activity["plannedstart"]) <= time()) {
            $status = "Overdue";
        }
        $activity['status'] = $status;
        if (isset($statusCounts[$status])) {
            $statusCounts[$status]++;
        }
    }
    unset($activity);

    $tiles = [
        ['label' => $activityFilter === 'all' ? 'Activities' : 'My activities', 'value' => count($data), 'icon' => 'bi-list-task', 'tone' => 'primary'],
        ['label' => 'In progress', 'value' => $statusCounts['In Progress'], 'icon' => 'bi-play-circle', 'tone' => 'primary'],
        ['label' => 'Overdue', 'value' => $statusCounts['Overdue'], 'icon' => 'bi-exclamation-triangle', 'tone' => 'danger'],
        ['label' => 'Completed', 'value' => $statusCounts['Completed'], 'icon' => 'bi-check2-circle', 'tone' => 'success'],
    ];

    $pageTitle = 'Dashboard';
    include 'includes/head.php';
?>
<body>
    <?php include 'includes/header.php'; ?>
    <main class="container app-main">
        <div class="page-header">
            <div>
                <h1 class="h3">Dashboard</h1>
            </div>
        </div>

        <?php if ($message): ?>
            <div class="alert alert-<?php echo $messageType; ?> alert-dismissible fade show" role="alert">
                <?php echo htmlspecialchars($message); ?>
                <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Close"></button>
            </div>
        <?php endif; ?>

        <!-- Projects -->
        <section class="dashboard-section">
            <div class="card">
                <div class="card-body">
                    <?php if (empty($projects)): ?>
                        <span class="text-body-secondary">You are not a member of any project.</span>
                    <?php else: ?>
                        <form method="get" class="row g-2 align-items-center">
                            <div class="col-auto">
                                <label for="projectid" class="col-form-label fw-medium">My Projects</label>
                            </div>
                            <div class="col-sm-8 col-lg-6">
                                <select class="form-select" id="projectid" name="projectid" onchange="this.form.submit()">
                                    <?php foreach ($projects as $project): ?>
                                        <option value="<?php echo (int)$project['projectid']; ?>" <?php echo (int)$project['projectid'] === $projectid ? 'selected' : ''; ?>>
                                            <?php echo htmlspecialchars($project['projectno'] . ' - ' . $project['projectname']); ?>
                                        </option>
                                    <?php endforeach; ?>
                                </select>
                            </div>
                            <input type="hidden" name="activities" value="<?php echo $activityFilter; ?>">
                            <input type="hidden" name="artefacts" value="<?php echo $artefactFilter; ?>">
                            <noscript>
                                <div class="col-auto"><button type="submit" class="btn btn-outline-secondary">Show</button></div>
                            </noscript>
                            <div class="col-auto">
                                <a href="project.php?projectid=<?php echo $projectid; ?>" class="btn btn-outline-secondary">
                                    <i class="bi bi-info-circle"></i> Project Details
                                </a>
                            </div>
                        </form>
                    <?php endif; ?>
                </div>
            </div>
        </section>

        <!-- Activities -->
        <section class="dashboard-section">
            <div class="d-flex align-items-center justify-content-between gap-2 mb-2">
                <h2 class="dashboard-section-title mb-0"><?php echo $activitiesTitle; ?></h2>
                <div class="d-flex gap-2">
                    <?php if ($isProjectManager): ?>
                        <button type="button" class="btn btn-sm btn-primary" data-bs-toggle="modal" data-bs-target="#addActivityModal">
                            <i class="bi bi-plus-lg"></i> Add Activity
                        </button>
                    <?php endif; ?>
                    <?php if ($projectid): ?>
                        <button type="button" class="btn btn-sm btn-outline-secondary" data-bs-toggle="modal" data-bs-target="#filterActivitiesModal">
                            <i class="bi bi-funnel"></i> Filter Activities
                        </button>
                    <?php endif; ?>
                </div>
            </div>
            <div class="row g-3 mb-3">
                <?php foreach ($tiles as $tile): ?>
                    <div class="col-6 col-lg-3">
                        <div class="card stat-tile">
                            <div class="card-body">
                                <span class="stat-icon status-<?php echo $tile['tone']; ?>"><i class="bi <?php echo $tile['icon']; ?>"></i></span>
                                <div>
                                    <div class="stat-value"><?php echo $tile['value']; ?></div>
                                    <div class="stat-label"><?php echo $tile['label']; ?></div>
                                </div>
                            </div>
                        </div>
                    </div>
                <?php endforeach; ?>
            </div>

            <div class="card">
                <?php if (empty($data)): ?>
                    <div class="empty-state">
                        <i class="bi bi-inbox"></i>
                        <?php echo $activityFilter === 'all' ? 'There are no activities in this project.' : 'You have no activities in this project.'; ?>
                    </div>
                <?php else: ?>
                    <div class="table-responsive">
                        <table class="table table-hover align-middle mb-0">
                            <thead>
                                <tr>
                                    <th>Activity</th>
                                    <th>Category</th>
                                    <th>Activity Manager</th>
                                    <th>Status</th>
                                </tr>
                            </thead>
                            <tbody>
                                <?php foreach ($data as $activity): ?>
                                    <?php $activityUrl = 'activity.php?activityid=' . urlencode($activity["activityid"]); ?>
                                    <tr class="clickable-row" data-href="<?php echo $activityUrl; ?>">
                                        <td class="fw-medium">
                                            <a href="<?php echo $activityUrl; ?>" class="text-reset text-decoration-none">
                                                <?php echo htmlspecialchars($activity["activityname"]); ?>
                                            </a>
                                        </td>
                                        <td class="text-body-secondary">
                                            <?php echo htmlspecialchars($activity["categoryname"] . " / " . $activity["subcategoryname"]); ?>
                                        </td>
                                        <td><?php echo htmlspecialchars($activity["activity_manager"]); ?></td>
                                        <td><?php echo StatusBadge::html($activity['status']); ?></td>
                                    </tr>
                                <?php endforeach; ?>
                            </tbody>
                        </table>
                    </div>
                <?php endif; ?>
            </div>
        </section>

        <!-- Artefacts -->
        <section class="dashboard-section">
            <div class="d-flex align-items-center justify-content-between gap-2 mb-2">
                <h2 class="dashboard-section-title mb-0"><?php echo $artefactsTitle; ?></h2>
                <div class="d-flex gap-2">
                    <?php if ($isProjectManager): ?>
                        <button type="button" class="btn btn-sm btn-primary" data-bs-toggle="modal" data-bs-target="#addArtefactTypeModal">
                            <i class="bi bi-plus-lg"></i> Add Type
                        </button>
                    <?php endif; ?>
                    <?php if ($projectid): ?>
                        <button type="button" class="btn btn-sm btn-outline-secondary" data-bs-toggle="modal" data-bs-target="#filterArtefactsModal">
                            <i class="bi bi-funnel"></i> Filter Artefacts
                        </button>
                    <?php endif; ?>
                </div>
            </div>
            <div class="card">
                <?php if (empty($artefactTypes)): ?>
                    <div class="empty-state">
                        <i class="bi bi-folder2-open"></i>
                        <?php echo $artefactFilter === 'all' ? 'No artefact types in this project.' : 'You have no artefacts in this project.'; ?>
                    </div>
                <?php else: ?>
                    <div class="table-responsive">
                        <table class="table table-hover align-middle mb-0">
                            <thead>
                                <tr>
                                    <th>Artefact Type</th>
                                    <th class="text-end">Artefacts</th>
                                    <th class="text-end">Completed</th>
                                    <th style="min-width: 10rem;">Cumulative Progress</th>
                                </tr>
                            </thead>
                            <tbody>
                                <?php foreach ($artefactTypes as $typeid => $type): ?>
                                    <?php $typeUrl = 'artefacts.php?projectid=' . $projectid . '&artefacttypeid=' . $typeid; ?>
                                    <tr class="clickable-row" data-href="<?php echo htmlspecialchars($typeUrl); ?>">
                                        <td class="fw-medium">
                                            <a href="<?php echo htmlspecialchars($typeUrl); ?>" class="text-reset text-decoration-none">
                                                <?php echo htmlspecialchars($type['name']); ?>
                                            </a>
                                        </td>
                                        <td class="text-end"><?php echo $type['count']; ?></td>
                                        <td class="text-end"><?php echo $type['completed']; ?></td>
                                        <td>
                                            <div class="d-flex align-items-center gap-2">
                                                <div class="progress flex-grow-1" role="progressbar"
                                                     aria-valuenow="<?php echo (float)$type['progress']; ?>" aria-valuemin="0" aria-valuemax="100">
                                                    <div class="progress-bar <?php echo $type['progress'] >= 100 ? 'is-complete' : ''; ?>"
                                                         style="width: <?php echo (float)$type['progress']; ?>%"></div>
                                                </div>
                                                <span class="small text-body-secondary" style="min-width: 2.5rem;"><?php echo number_format($type['progress'], 0); ?>%</span>
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

    <?php if ($projectid): ?>
    <!-- Filter Activities Modal -->
    <div class="modal fade" id="filterActivitiesModal" tabindex="-1" aria-labelledby="filterActivitiesModalLabel" aria-hidden="true">
        <div class="modal-dialog modal-sm">
            <div class="modal-content">
                <form method="get" action="dashboard.php">
                    <input type="hidden" name="projectid" value="<?php echo $projectid; ?>">
                    <input type="hidden" name="artefacts" value="<?php echo $artefactFilter; ?>">
                    <div class="modal-header">
                        <h5 class="modal-title" id="filterActivitiesModalLabel">Filter Activities</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <div class="form-check">
                            <input class="form-check-input" type="radio" name="activities" id="activitiesMine" value="mine" <?php echo $activityFilter === 'mine' ? 'checked' : ''; ?>>
                            <label class="form-check-label" for="activitiesMine">My Activities</label>
                            <div class="form-text mt-0">Activities you manage or have been assigned a task in.</div>
                        </div>
                        <div class="form-check mt-2">
                            <input class="form-check-input" type="radio" name="activities" id="activitiesAll" value="all" <?php echo $activityFilter === 'all' ? 'checked' : ''; ?>>
                            <label class="form-check-label" for="activitiesAll">All Activities</label>
                            <div class="form-text mt-0">Every activity in this project.</div>
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <button type="submit" class="btn btn-primary">Apply</button>
                    </div>
                </form>
            </div>
        </div>
    </div>

    <!-- Filter Artefacts Modal -->
    <div class="modal fade" id="filterArtefactsModal" tabindex="-1" aria-labelledby="filterArtefactsModalLabel" aria-hidden="true">
        <div class="modal-dialog modal-sm">
            <div class="modal-content">
                <form method="get" action="dashboard.php">
                    <input type="hidden" name="projectid" value="<?php echo $projectid; ?>">
                    <input type="hidden" name="activities" value="<?php echo $activityFilter; ?>">
                    <div class="modal-header">
                        <h5 class="modal-title" id="filterArtefactsModalLabel">Filter Artefacts</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <div class="form-check">
                            <input class="form-check-input" type="radio" name="artefacts" id="artefactsMine" value="mine" <?php echo $artefactFilter === 'mine' ? 'checked' : ''; ?>>
                            <label class="form-check-label" for="artefactsMine">My Artefacts</label>
                            <div class="form-text mt-0">Artefacts you own.</div>
                        </div>
                        <div class="form-check mt-2">
                            <input class="form-check-input" type="radio" name="artefacts" id="artefactsAll" value="all" <?php echo $artefactFilter === 'all' ? 'checked' : ''; ?>>
                            <label class="form-check-label" for="artefactsAll">All Artefacts</label>
                            <div class="form-text mt-0">Every artefact in this project.</div>
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <button type="submit" class="btn btn-primary">Apply</button>
                    </div>
                </form>
            </div>
        </div>
    </div>
    <?php endif; ?>

    <?php if ($isProjectManager): ?>
    <!-- Add Activity Modal -->
    <div class="modal fade" id="addActivityModal" tabindex="-1" aria-labelledby="addActivityModalLabel" aria-hidden="true">
        <div class="modal-dialog modal-lg">
            <div class="modal-content">
                <form method="POST" action="dashboard.php?<?php echo htmlspecialchars($dashboardQuery); ?>">
                    <?php echo Csrf::field(); ?>
                    <div class="modal-header">
                        <h5 class="modal-title" id="addActivityModalLabel">Add Activity</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                            <div class="mb-3">
                                <label for="activityname" class="form-label">Activity name</label>
                                <input type="text" class="form-control" id="activityname" name="activityname" maxlength="255" required>
                            </div>
                            <div class="mb-3">
                                <label for="activitydescription" class="form-label">Description</label>
                                <textarea class="form-control" id="activitydescription" name="activitydescription" rows="3" maxlength="1000"></textarea>
                            </div>
                            <div class="row g-3 mb-3">
                                <div class="col-sm-6">
                                    <label for="subcategoryid" class="form-label">Category</label>
                                    <select class="form-select" id="subcategoryid" name="subcategoryid" required>
                                        <option value="" <?php echo empty($subcategories) ? '' : 'selected'; ?>>Please select a category...</option>
                                        <option value="new" <?php echo empty($subcategories) ? 'selected' : ''; ?>>+ New category...</option>
                                        <?php foreach (array_unique(array_column($subcategories, 'categoryname')) as $categoryname): ?>
                                            <optgroup label="<?php echo htmlspecialchars($categoryname); ?>">
                                                <?php foreach ($subcategories as $subcategory): ?>
                                                    <?php if ($subcategory['categoryname'] === $categoryname): ?>
                                                        <option value="<?php echo (int)$subcategory['subcategoryid']; ?>"><?php echo htmlspecialchars($subcategory['subcategoryname']); ?></option>
                                                    <?php endif; ?>
                                                <?php endforeach; ?>
                                            </optgroup>
                                        <?php endforeach; ?>
                                    </select>
                                </div>
                                <div class="col-sm-6">
                                    <label for="activitymanagerid" class="form-label">Activity manager</label>
                                    <select class="form-select" id="activitymanagerid" name="activitymanagerid" required>
                                        <option value="">Please select a user...</option>
                                        <?php foreach ($teamMembers as $member): ?>
                                            <option value="<?php echo (int)$member['employeeid']; ?>"><?php echo htmlspecialchars($member['fullname']); ?></option>
                                        <?php endforeach; ?>
                                    </select>
                                </div>
                            </div>
                            <div class="row g-3 mb-3" id="newCategoryFields" <?php echo empty($subcategories) ? '' : 'hidden'; ?>>
                                <div class="col-sm-6">
                                    <label for="newcategoryname" class="form-label">Category name</label>
                                    <input type="text" class="form-control" id="newcategoryname" name="newcategoryname" maxlength="255" list="categoryOptions"
                                           placeholder="Existing or new category">
                                    <datalist id="categoryOptions">
                                        <?php foreach ($categories as $category): ?>
                                            <option value="<?php echo htmlspecialchars($category['categoryname']); ?>"></option>
                                        <?php endforeach; ?>
                                    </datalist>
                                </div>
                                <div class="col-sm-6">
                                    <label for="newsubcategoryname" class="form-label">Subcategory name</label>
                                    <input type="text" class="form-control" id="newsubcategoryname" name="newsubcategoryname" maxlength="255">
                                </div>
                                <div class="col-12 form-text mt-1">Existing names are reused, so picking an existing category adds the subcategory under it.</div>
                            </div>
                            <div class="section-label mb-2">Planned</div>
                            <div class="row g-3">
                                <div class="col-sm-4">
                                    <label for="plannedstart" class="form-label small">Start</label>
                                    <input type="date" class="form-control" id="plannedstart" name="plannedstart">
                                </div>
                                <div class="col-sm-4">
                                    <label for="plannedfinish" class="form-label small">Finish</label>
                                    <input type="date" class="form-control" id="plannedfinish" name="plannedfinish">
                                </div>
                                <div class="col-sm-4">
                                    <label for="plannedhours" class="form-label small">Hours</label>
                                    <input type="number" step="0.01" min="0" class="form-control" id="plannedhours" name="plannedhours">
                                </div>
                            </div>
                            <div class="form-text">Forecast dates and hours start out equal to the plan.</div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <button type="submit" name="add_activity" class="btn btn-primary">Add Activity</button>
                    </div>
                </form>
            </div>
        </div>
    </div>

    <!-- Add Artefact Type Modal -->
    <div class="modal fade" id="addArtefactTypeModal" tabindex="-1" aria-labelledby="addArtefactTypeModalLabel" aria-hidden="true">
        <div class="modal-dialog modal-xl">
            <div class="modal-content">
                <form method="POST" action="dashboard.php?<?php echo htmlspecialchars($dashboardQuery); ?>">
                    <?php echo Csrf::field(); ?>
                    <div class="modal-header">
                        <h5 class="modal-title" id="addArtefactTypeModalLabel">Add Artefact Type</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <div class="mb-3">
                            <label for="artefactname" class="form-label">Type name</label>
                            <input type="text" class="form-control" id="artefactname" name="artefactname" maxlength="255"
                                   placeholder="Enter artefact type name..." required>
                        </div>
                        <div>
                            <label for="artefacttypedescription" class="form-label">Description</label>
                            <textarea class="form-control" id="artefacttypedescription" name="artefacttypedescription" rows="3" maxlength="1000"></textarea>
                        </div>
                        <div class="mt-4">
                            <div class="d-flex justify-content-between align-items-center mb-2">
                                <span class="form-label mb-0">Data fields</span>
                                <button type="button" class="btn btn-sm btn-outline-secondary" id="addDataFieldButton">
                                    <i class="bi bi-plus-lg"></i> Add Field
                                </button>
                            </div>
                            <p class="small text-body-secondary mb-2" id="noDataFieldsText">No data fields. Artefacts of this type will only have a title and description.</p>
                            <table class="table table-sm align-middle mb-0" id="dataFieldsTable" hidden>
                                <thead>
                                    <tr>
                                        <th>Field title</th>
                                        <th style="width: 8rem;">Type</th>
                                        <th style="width: 7rem;">Max length</th>
                                        <th style="width: 7rem;">Min value</th>
                                        <th style="width: 7rem;">Max value</th>
                                        <th style="width: 6rem;">Editable</th>
                                        <th style="width: 2.5rem;"></th>
                                    </tr>
                                </thead>
                                <tbody id="dataFieldsBody"></tbody>
                            </table>
                            <template id="dataFieldRowTemplate">
                                <tr>
                                    <td><input type="text" class="form-control form-control-sm" name="fields[fieldtitle][]" maxlength="255" aria-label="Field title" required></td>
                                    <td>
                                        <select class="form-select form-select-sm" name="fields[valuetype][]" aria-label="Value type" data-role="valuetype">
                                            <option value="0">Text</option>
                                            <option value="1">Number</option>
                                        </select>
                                    </td>
                                    <td><input type="number" class="form-control form-control-sm" name="fields[maximumlength][]" min="1" max="2000" step="1" aria-label="Maximum length"></td>
                                    <td><input type="number" class="form-control form-control-sm" name="fields[minimumvalue][]" step="1" aria-label="Minimum value" data-role="numeric" readonly></td>
                                    <td><input type="number" class="form-control form-control-sm" name="fields[maximumvalue][]" step="1" aria-label="Maximum value" data-role="numeric" readonly></td>
                                    <td>
                                        <select class="form-select form-select-sm" name="fields[editable][]" aria-label="Editable">
                                            <option value="1">Yes</option>
                                            <option value="0">No</option>
                                        </select>
                                    </td>
                                    <td class="text-end">
                                        <button type="button" class="btn btn-sm btn-outline-danger" data-role="remove" aria-label="Remove field">
                                            <i class="bi bi-x-lg"></i>
                                        </button>
                                    </td>
                                </tr>
                            </template>
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <button type="submit" name="add_artefact_type" class="btn btn-primary">Add Artefact Type</button>
                    </div>
                </form>
            </div>
        </div>
    </div>

    <?php endif; ?>

    <?php include 'includes/footer.php'; ?>
    <script>
    document.querySelectorAll('.clickable-row').forEach(function (row) {
        row.addEventListener('click', function (e) {
            if (!e.target.closest('a, button')) {
                window.location.href = row.dataset.href;
            }
        });
    });

    // Show the new category/subcategory fields when "+ New category..." is selected
    var subcategorySelect = document.getElementById('subcategoryid');
    if (subcategorySelect) {
        var toggleNewCategory = function () {
            var isNew = subcategorySelect.value === 'new';
            document.getElementById('newCategoryFields').hidden = !isNew;
            document.getElementById('newcategoryname').required = isNew;
            document.getElementById('newsubcategoryname').required = isNew;
        };
        subcategorySelect.addEventListener('change', toggleNewCategory);
        toggleNewCategory();
    }

    // Add/remove data field rows in the Add Artefact Type modal
    var dataFieldsBody = document.getElementById('dataFieldsBody');
    if (dataFieldsBody) {
        var dataFieldRowTemplate = document.getElementById('dataFieldRowTemplate');
        var updateDataFieldsVisibility = function () {
            var hasRows = dataFieldsBody.rows.length > 0;
            document.getElementById('dataFieldsTable').hidden = !hasRows;
            document.getElementById('noDataFieldsText').hidden = hasRows;
        };
        document.getElementById('addDataFieldButton').addEventListener('click', function () {
            var row = dataFieldRowTemplate.content.firstElementChild.cloneNode(true);
            var valuetype = row.querySelector('[data-role="valuetype"]');
            // Min/max values only apply to number fields
            valuetype.addEventListener('change', function () {
                var isNumber = valuetype.value === '1';
                row.querySelectorAll('[data-role="numeric"]').forEach(function (input) {
                    input.readOnly = !isNumber;
                    if (!isNumber) {
                        input.value = '';
                    }
                });
            });
            row.querySelector('[data-role="remove"]').addEventListener('click', function () {
                row.remove();
                updateDataFieldsVisibility();
            });
            dataFieldsBody.appendChild(row);
            updateDataFieldsVisibility();
            row.querySelector('input').focus();
        });
    }
    </script>
</body>
</html>
