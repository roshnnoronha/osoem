<?php
    require_once '../src/bootstrap.php';

    $activityService = new ActivityService();
    $projectService = new ProjectService();

    $projectid = (int)($_GET['projectid'] ?? 0);

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

    $milestoneService = new MilestoneService();
    $isProjectManager = $projectService->isProjectManager($projectid, $eid);

    $message = '';
    $messageType = '';

    // Validates the posted milestone name and steps. Returns [name, steps, error]; steps are
    // ['milestonestepid' => int|null, 'name' => string, 'ratio' => float] sorted by progress ratio.
    function parseMilestoneInput($postedName, $postedSteps) {
        $name = trim((string)$postedName);
        if ($name === '' || mb_strlen($name) > 255) {
            return [null, null, 'Please enter a milestone name of at most 255 characters.'];
        }
        if (!is_array($postedSteps) || empty($postedSteps)) {
            return [null, null, 'A milestone needs at least one step.'];
        }

        $steps = [];
        foreach ($postedSteps as $postedStep) {
            if (!is_array($postedStep)) {
                return [null, null, 'Invalid milestone steps.'];
            }
            $stepName = trim((string)($postedStep['name'] ?? ''));
            $ratio = $postedStep['ratio'] ?? '';
            $stepid = $postedStep['id'] ?? '';

            if ($stepName === '' || mb_strlen($stepName) > 255) {
                return [null, null, 'Each step needs a name of at most 255 characters.'];
            }
            $ratio = is_numeric($ratio) ? round((float)$ratio, 2) : null;
            if ($ratio === null || $ratio <= 0 || $ratio > 1) {
                return [null, null, "The progress ratio of step \"$stepName\" must be between 0.01 and 1.00."];
            }
            $steps[] = [
                'milestonestepid' => ctype_digit((string)$stepid) ? (int)$stepid : null,
                'name' => $stepName,
                'ratio' => $ratio,
            ];
        }

        usort($steps, function ($a, $b) { return $a['ratio'] <=> $b['ratio']; });
        $ratios = array_column($steps, 'ratio');
        if (count(array_unique(array_map('strval', $ratios))) !== count($ratios)) {
            return [null, null, 'Each step must have a different progress ratio.'];
        }
        if (end($ratios) != 1.0) {
            return [null, null, 'The final step must have a progress ratio of 1.00.'];
        }

        return [$name, $steps, null];
    }

    $milestoneActions = ['add_milestone', 'edit_milestone'];
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && array_intersect(array_keys($_POST), $milestoneActions)) {
        if (!Csrf::validate($_POST['csrf_token'] ?? '')) {
            http_response_code(403);
            die('Invalid or expired form submission (CSRF token mismatch). Please refresh the page and try again.');
        }

        $projectMilestones = $milestoneService->getMilestonesForProject($projectid);

        if (!$isProjectManager) {
            $message = 'Only the project manager can manage milestones.';
            $messageType = 'danger';
        } elseif (isset($_POST['add_milestone'])) {
            [$name, $steps, $error] = parseMilestoneInput($_POST['milestonename'] ?? '', $_POST['steps'] ?? []);
            foreach ($steps ?? [] as &$step) {
                $step['milestonestepid'] = null;
            }
            unset($step);

            if ($error) {
                $message = $error;
                $messageType = 'warning';
            } elseif ($milestoneService->createMilestone($projectid, $name, $steps)) {
                $message = "Milestone \"$name\" added.";
                $messageType = 'success';
            } else {
                $message = 'Error adding the milestone. Please try again.';
                $messageType = 'danger';
            }
        } elseif (isset($_POST['edit_milestone'])) {
            $milestoneid = (int)($_POST['milestoneid'] ?? 0);
            [$name, $steps, $error] = parseMilestoneInput($_POST['milestonename'] ?? '', $_POST['steps'] ?? []);

            if (!isset($projectMilestones[$milestoneid])) {
                $message = 'Please select a milestone in this project.';
                $messageType = 'warning';
            } elseif ($error) {
                $message = $error;
                $messageType = 'warning';
            } else {
                // Steps cannot be removed, so every existing step must still be present
                $keptids = array_column($steps, 'milestonestepid');
                $existingids = array_map('intval', array_column($projectMilestones[$milestoneid]['steps'], 'milestonestepid'));

                if (array_diff($existingids, $keptids)) {
                    $message = 'Existing milestone steps cannot be removed.';
                    $messageType = 'warning';
                } elseif ($milestoneService->updateMilestone($milestoneid, $name, $steps)) {
                    $message = "Milestone \"$name\" updated.";
                    $messageType = 'success';
                } else {
                    $message = 'Error updating the milestone. Please try again.';
                    $messageType = 'danger';
                }
            }
        }
    }

    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['add_team_member'])) {
        if (!Csrf::validate($_POST['csrf_token'] ?? '')) {
            http_response_code(403);
            die('Invalid or expired form submission (CSRF token mismatch). Please refresh the page and try again.');
        }

        $newmemberid = (int)($_POST['employeeid'] ?? 0);
        $role = $_POST['role'] ?? '';
        $candidates = array_column($projectService->getNonTeamMembers($projectid), 'fullname', 'employeeid');

        if (!$isProjectManager) {
            $message = 'Only the project manager can add team members.';
            $messageType = 'danger';
        } elseif (!isset($candidates[$newmemberid])) {
            $message = 'Please select an employee who is not already in this project team.';
            $messageType = 'warning';
        } elseif (!ctype_digit((string)$role) || !array_key_exists((int)$role, ProjectService::ROLE_NAMES)) {
            $message = 'Please select a valid role.';
            $messageType = 'warning';
        } elseif ($projectService->addTeamMember($projectid, $newmemberid, (int)$role)) {
            $message = $candidates[$newmemberid] . ' added to the project team.';
            $messageType = 'success';
        } else {
            $message = 'Error adding the team member. Please try again.';
            $messageType = 'danger';
        }
    }

    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['edit_project'])) {
        if (!Csrf::validate($_POST['csrf_token'] ?? '')) {
            http_response_code(403);
            die('Invalid or expired form submission (CSRF token mismatch). Please refresh the page and try again.');
        }

        $projectno = trim((string)($_POST['projectno'] ?? ''));
        $projectname = trim((string)($_POST['projectname'] ?? ''));

        if (!$isProjectManager) {
            $message = 'Only the project manager can edit the project details.';
            $messageType = 'danger';
        } elseif ($projectno === '' || mb_strlen($projectno) > 255) {
            $message = 'Please enter a project number of at most 255 characters.';
            $messageType = 'warning';
        } elseif ($projectname === '' || mb_strlen($projectname) > 255) {
            $message = 'Please enter a project name of at most 255 characters.';
            $messageType = 'warning';
        } elseif ($projectService->projectNumberExists($projectno, $projectid)) {
            $message = "Project number \"$projectno\" is already used by another project.";
            $messageType = 'warning';
        } elseif ($projectService->updateProject($projectid, $projectno, $projectname)) {
            $project['projectno'] = $projectno;
            $project['projectname'] = $projectname;
            $message = 'Project details updated.';
            $messageType = 'success';
        } else {
            $message = 'Error updating the project details. Please try again.';
            $messageType = 'danger';
        }
    }

    $milestones = $milestoneService->getMilestonesForProject($projectid);

    $teamMembers = $projectService->getProjectTeamMembers($projectid);
    $nonTeamMembers = $isProjectManager ? $projectService->getNonTeamMembers($projectid) : [];
    $projectManagers = array_values(array_filter(
        $teamMembers,
        function ($member) { return (int)$member['role'] === 2; }
    ));

    $pageTitle = $project['projectname'];
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
                        <li class="breadcrumb-item active" aria-current="page">Project</li>
                    </ol>
                </nav>
                <div class="section-label">Project</div>
                <h1 class="h3"><?php echo htmlspecialchars($project['projectname']); ?></h1>
                <p class="page-subtitle"><?php echo htmlspecialchars($project['projectno']); ?></p>
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
                        <?php if ($isProjectManager): ?>
                            <div class="card-actions">
                                <button type="button" class="btn btn-sm btn-outline-secondary" data-bs-toggle="modal" data-bs-target="#editProjectModal">
                                    <i class="bi bi-pencil"></i> Edit Details
                                </button>
                            </div>
                        <?php endif; ?>
                    </div>
                    <div class="card-body">
                        <dl class="row detail-list">
                            <dt class="col-sm-4 col-lg-3">Project number</dt>
                            <dd class="col-sm-8 col-lg-9"><?php echo htmlspecialchars($project['projectno']); ?></dd>
                            <dt class="col-sm-4 col-lg-3">Project name</dt>
                            <dd class="col-sm-8 col-lg-9"><?php echo htmlspecialchars($project['projectname']); ?></dd>
                            <dt class="col-sm-4 col-lg-3">Project manager</dt>
                            <dd class="col-sm-8 col-lg-9">
                                <?php if (empty($projectManagers)): ?>
                                    <span class="text-body-secondary">Not assigned</span>
                                <?php else: ?>
                                    <?php foreach ($projectManagers as $manager): ?>
                                        <span class="d-block"><?php echo htmlspecialchars($manager['fullname']); ?></span>
                                    <?php endforeach; ?>
                                <?php endif; ?>
                            </dd>
                            <dt class="col-sm-4 col-lg-3">Team size</dt>
                            <dd class="col-sm-8 col-lg-9"><?php echo count($teamMembers); ?></dd>
                        </dl>
                    </div>
                </div>
            </div>

            <div class="col-12">
                <div class="card">
                    <div class="card-header">
                        <h2 class="card-title">Team members</h2>
                        <?php if ($isProjectManager): ?>
                            <div class="card-actions">
                                <button type="button" class="btn btn-sm btn-primary" data-bs-toggle="modal" data-bs-target="#addTeamMemberModal"<?php echo empty($nonTeamMembers) ? ' disabled title="All employees are already in this project team"' : ''; ?>>
                                    <i class="bi bi-person-plus"></i> Add Member
                                </button>
                            </div>
                        <?php endif; ?>
                    </div>
                    <?php if (empty($teamMembers)): ?>
                        <div class="empty-state">
                            <i class="bi bi-people"></i>
                            No team members in this project.
                        </div>
                    <?php else: ?>
                        <div class="table-responsive">
                            <table class="table align-middle mb-0">
                                <thead>
                                    <tr>
                                        <th>Name</th>
                                        <th>Role</th>
                                        <th>Department</th>
                                        <th>Email</th>
                                    </tr>
                                </thead>
                                <tbody>
                                    <?php foreach ($teamMembers as $member): ?>
                                        <tr>
                                            <td class="fw-medium"><?php echo htmlspecialchars($member['fullname']); ?></td>
                                            <td><?php echo htmlspecialchars(ProjectService::roleName($member['role'])); ?></td>
                                            <td class="text-body-secondary"><?php echo htmlspecialchars($member['departmentname'] ?? ''); ?></td>
                                            <td>
                                                <a href="mailto:<?php echo htmlspecialchars($member['email']); ?>"><?php echo htmlspecialchars($member['email']); ?></a>
                                            </td>
                                        </tr>
                                    <?php endforeach; ?>
                                </tbody>
                            </table>
                        </div>
                    <?php endif; ?>
                </div>
            </div>

            <div class="col-12">
                <div class="card">
                    <div class="card-header">
                        <h2 class="card-title">Milestones</h2>
                        <?php if ($isProjectManager): ?>
                            <div class="card-actions">
                                <button type="button" class="btn btn-sm btn-primary" data-bs-toggle="modal" data-bs-target="#addMilestoneModal">
                                    <i class="bi bi-plus-lg"></i> Add Milestone
                                </button>
                                <?php if (!empty($milestones)): ?>
                                    <button type="button" class="btn btn-sm btn-outline-secondary milestone-single-action" data-bs-toggle="modal" data-bs-target="#editMilestoneModal" disabled>
                                        <i class="bi bi-pencil"></i> Edit Milestone
                                    </button>
                                <?php endif; ?>
                            </div>
                        <?php endif; ?>
                    </div>
                    <?php if (empty($milestones)): ?>
                        <div class="empty-state">
                            <i class="bi bi-flag"></i>
                            No milestones in this project.
                        </div>
                    <?php else: ?>
                        <div class="table-responsive">
                            <table class="table align-middle mb-0">
                                <thead>
                                    <tr>
                                        <?php if ($isProjectManager): ?>
                                            <th class="row-select" style="width: 1%;"></th>
                                        <?php endif; ?>
                                        <th>Milestone</th>
                                        <th>Steps</th>
                                        <th class="text-end">Tracking artefacts</th>
                                    </tr>
                                </thead>
                                <tbody>
                                    <?php foreach ($milestones as $milestone): ?>
                                        <?php
                                            $milestoneJson = json_encode([
                                                'id' => $milestone['milestoneid'],
                                                'name' => $milestone['milestonename'],
                                                'steps' => array_map(function ($step) {
                                                    return [
                                                        'id' => (int)$step['milestonestepid'],
                                                        'name' => $step['milestonestepname'],
                                                        'ratio' => $step['progressratio'],
                                                    ];
                                                }, $milestone['steps']),
                                            ]);
                                        ?>
                                        <tr>
                                            <?php if ($isProjectManager): ?>
                                                <td class="row-select">
                                                    <input class="form-check-input milestone-select" type="checkbox"
                                                           value="<?php echo (int)$milestone['milestoneid']; ?>"
                                                           data-milestone="<?php echo htmlspecialchars($milestoneJson, ENT_QUOTES); ?>"
                                                           aria-label="Select <?php echo htmlspecialchars($milestone['milestonename'], ENT_QUOTES); ?>">
                                                </td>
                                            <?php endif; ?>
                                            <td class="fw-medium"><?php echo htmlspecialchars($milestone['milestonename']); ?></td>
                                            <td>
                                                <?php if (empty($milestone['steps'])): ?>
                                                    <span class="text-body-secondary">&mdash;</span>
                                                <?php else: ?>
                                                    <div class="d-flex flex-wrap gap-1">
                                                        <?php foreach ($milestone['steps'] as $step): ?>
                                                            <span class="badge text-bg-light border fw-normal">
                                                                <?php echo htmlspecialchars($step['milestonestepname']); ?>
                                                                <span class="text-body-secondary"><?php echo htmlspecialchars($step['progressratio']); ?></span>
                                                            </span>
                                                        <?php endforeach; ?>
                                                    </div>
                                                <?php endif; ?>
                                            </td>
                                            <td class="text-end"><?php echo (int)$milestone['artefactcount']; ?></td>
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

    <?php if ($isProjectManager): ?>
    <!-- Edit Project Modal -->
    <div class="modal fade" id="editProjectModal" tabindex="-1" aria-labelledby="editProjectModalLabel" aria-hidden="true">
        <div class="modal-dialog">
            <div class="modal-content">
                <form method="POST">
                    <?php echo Csrf::field(); ?>
                    <div class="modal-header">
                        <h5 class="modal-title" id="editProjectModalLabel">Edit Project</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <div class="mb-3">
                            <label for="editProjectNo" class="form-label">Project number</label>
                            <input type="text" class="form-control" id="editProjectNo" name="projectno" maxlength="255" required
                                   value="<?php echo htmlspecialchars($project['projectno'], ENT_QUOTES); ?>">
                        </div>
                        <div>
                            <label for="editProjectName" class="form-label">Project name</label>
                            <input type="text" class="form-control" id="editProjectName" name="projectname" maxlength="255" required
                                   value="<?php echo htmlspecialchars($project['projectname'], ENT_QUOTES); ?>">
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <button type="submit" name="edit_project" class="btn btn-primary">Save</button>
                    </div>
                </form>
            </div>
        </div>
    </div>

    <!-- Add Team Member Modal -->
    <div class="modal fade" id="addTeamMemberModal" tabindex="-1" aria-labelledby="addTeamMemberModalLabel" aria-hidden="true">
        <div class="modal-dialog">
            <div class="modal-content">
                <form method="POST">
                    <?php echo Csrf::field(); ?>
                    <div class="modal-header">
                        <h5 class="modal-title" id="addTeamMemberModalLabel">Add Team Member</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <div class="mb-3">
                            <label for="addTeamMemberEmployee" class="form-label">Employee</label>
                            <select class="form-select" id="addTeamMemberEmployee" name="employeeid" required>
                                <option value="" selected disabled>Select an employee</option>
                                <?php foreach ($nonTeamMembers as $employee): ?>
                                    <option value="<?php echo (int)$employee['employeeid']; ?>">
                                        <?php echo htmlspecialchars($employee['fullname']); ?>
                                        <?php if (!empty($employee['departmentname'])): ?>
                                            (<?php echo htmlspecialchars($employee['departmentname']); ?>)
                                        <?php endif; ?>
                                    </option>
                                <?php endforeach; ?>
                            </select>
                        </div>
                        <div>
                            <label for="addTeamMemberRole" class="form-label">Role</label>
                            <select class="form-select" id="addTeamMemberRole" name="role" required>
                                <?php foreach (ProjectService::ROLE_NAMES as $roleValue => $roleName): ?>
                                    <option value="<?php echo (int)$roleValue; ?>"<?php echo $roleValue === 0 ? ' selected' : ''; ?>>
                                        <?php echo htmlspecialchars($roleName); ?>
                                    </option>
                                <?php endforeach; ?>
                            </select>
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <button type="submit" name="add_team_member" class="btn btn-primary">Add</button>
                    </div>
                </form>
            </div>
        </div>
    </div>

    <!-- Add Milestone Modal -->
    <div class="modal fade" id="addMilestoneModal" tabindex="-1" aria-labelledby="addMilestoneModalLabel" aria-hidden="true">
        <div class="modal-dialog modal-lg">
            <div class="modal-content">
                <form method="POST">
                    <?php echo Csrf::field(); ?>
                    <div class="modal-header">
                        <h5 class="modal-title" id="addMilestoneModalLabel">Add Milestone</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <div class="mb-3">
                            <label for="addMilestoneModalName" class="form-label">Milestone name</label>
                            <input type="text" class="form-control milestone-name" id="addMilestoneModalName" name="milestonename" maxlength="255" required>
                        </div>
                        <div class="row g-2 small text-body-secondary mb-1">
                            <div class="col">Step</div>
                            <div class="col-3">Progress ratio</div>
                            <div class="col-auto" style="width: 2.5rem;"></div>
                        </div>
                        <div class="milestone-steps"></div>
                        <button type="button" class="btn btn-sm btn-outline-secondary add-step">
                            <i class="bi bi-plus-lg"></i> Add Step
                        </button>
                        <div class="form-text">Progress ratios are cumulative (e.g. 0.50, 0.75, 1.00); the final step must be 1.00.</div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <button type="submit" name="add_milestone" class="btn btn-primary">Add</button>
                    </div>
                </form>
            </div>
        </div>
    </div>

    <!-- Edit Milestone Modal -->
    <div class="modal fade" id="editMilestoneModal" tabindex="-1" aria-labelledby="editMilestoneModalLabel" aria-hidden="true">
        <div class="modal-dialog modal-lg">
            <div class="modal-content">
                <form method="POST">
                    <?php echo Csrf::field(); ?>
                    <input type="hidden" name="milestoneid" class="milestone-id">
                    <div class="modal-header">
                        <h5 class="modal-title" id="editMilestoneModalLabel">Edit Milestone</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        <div class="mb-3">
                            <label for="editMilestoneModalName" class="form-label">Milestone name</label>
                            <input type="text" class="form-control milestone-name" id="editMilestoneModalName" name="milestonename" maxlength="255" required>
                        </div>
                        <div class="row g-2 small text-body-secondary mb-1">
                            <div class="col">Step</div>
                            <div class="col-3">Progress ratio</div>
                            <div class="col-auto" style="width: 2.5rem;"></div>
                        </div>
                        <div class="milestone-steps"></div>
                        <button type="button" class="btn btn-sm btn-outline-secondary add-step">
                            <i class="bi bi-plus-lg"></i> Add Step
                        </button>
                        <div class="form-text">Progress ratios are cumulative (e.g. 0.50, 0.75, 1.00); the final step must be 1.00.</div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-outline-secondary" data-bs-dismiss="modal">Cancel</button>
                        <button type="submit" name="edit_milestone" class="btn btn-primary">Save</button>
                    </div>
                </form>
            </div>
        </div>
    </div>

    <?php endif; ?>

    <?php include 'includes/footer.php'; ?>
    <?php if ($isProjectManager): ?>
    <script>
    // Milestone selection: Edit needs exactly one milestone
    var milestoneCheckboxes = Array.prototype.slice.call(document.querySelectorAll('.milestone-select'));

    function selectedMilestones() {
        return milestoneCheckboxes.filter(function (cb) { return cb.checked; });
    }

    function updateMilestoneSelection() {
        var count = selectedMilestones().length;
        document.querySelectorAll('.milestone-single-action').forEach(function (btn) {
            btn.disabled = count !== 1;
        });
    }

    milestoneCheckboxes.forEach(function (cb) { cb.addEventListener('change', updateMilestoneSelection); });

    // Step editor shared by the Add and Edit modals. Each row posts steps[n][id|name|ratio].
    // Saved steps cannot be removed; only rows added in the form (and not yet saved) can be.
    var stepCounter = 0;

    function updateStepRemoveButtons(container) {
        var rows = container.querySelectorAll('.milestone-step-row');
        rows.forEach(function (row) {
            var removeBtn = row.querySelector('.remove-step');
            if (removeBtn) removeBtn.disabled = rows.length === 1;
        });
    }

    function addStepRow(container, step) {
        var index = stepCounter++;
        var row = document.createElement('div');
        row.className = 'row g-2 align-items-center mb-2 milestone-step-row';

        var idInput = document.createElement('input');
        idInput.type = 'hidden';
        idInput.name = 'steps[' + index + '][id]';
        idInput.value = step.id || '';
        row.appendChild(idInput);

        var nameCol = document.createElement('div');
        nameCol.className = 'col';
        var nameInput = document.createElement('input');
        nameInput.type = 'text';
        nameInput.className = 'form-control';
        nameInput.name = 'steps[' + index + '][name]';
        nameInput.maxLength = 255;
        nameInput.required = true;
        nameInput.placeholder = 'Step name';
        nameInput.setAttribute('aria-label', 'Step name');
        nameInput.value = step.name || '';
        nameCol.appendChild(nameInput);
        row.appendChild(nameCol);

        var ratioCol = document.createElement('div');
        ratioCol.className = 'col-3';
        var ratioInput = document.createElement('input');
        ratioInput.type = 'number';
        ratioInput.className = 'form-control';
        ratioInput.name = 'steps[' + index + '][ratio]';
        ratioInput.min = '0.01';
        ratioInput.max = '1';
        ratioInput.step = '0.01';
        ratioInput.required = true;
        ratioInput.setAttribute('aria-label', 'Progress ratio');
        ratioInput.value = step.ratio || '';
        ratioCol.appendChild(ratioInput);
        row.appendChild(ratioCol);

        var removeCol = document.createElement('div');
        removeCol.className = 'col-auto';
        removeCol.style.width = '2.5rem';
        if (!step.id) {
            var removeBtn = document.createElement('button');
            removeBtn.type = 'button';
            removeBtn.className = 'btn btn-sm btn-outline-danger btn-icon remove-step';
            removeBtn.title = 'Remove step';
            removeBtn.setAttribute('aria-label', 'Remove step');
            removeBtn.innerHTML = '<i class="bi bi-x-lg"></i>';
            removeBtn.addEventListener('click', function () {
                row.remove();
                updateStepRemoveButtons(container);
            });
            removeCol.appendChild(removeBtn);
        }
        row.appendChild(removeCol);

        container.appendChild(row);
        updateStepRemoveButtons(container);
    }

    document.querySelectorAll('#addMilestoneModal, #editMilestoneModal').forEach(function (modal) {
        var container = modal.querySelector('.milestone-steps');
        modal.querySelector('.add-step').addEventListener('click', function () {
            addStepRow(container, {});
            container.lastElementChild.querySelector('input[type="text"]').focus();
        });
    });

    document.getElementById('addMilestoneModal').addEventListener('show.bs.modal', function () {
        var modal = this;
        modal.querySelector('form').reset();
        var container = modal.querySelector('.milestone-steps');
        container.innerHTML = '';
        addStepRow(container, { ratio: '1.00' });
    });

    var editMilestoneModal = document.getElementById('editMilestoneModal');
    if (editMilestoneModal) {
        editMilestoneModal.addEventListener('show.bs.modal', function () {
            var selected = selectedMilestones();
            if (selected.length !== 1) return;
            var milestone = JSON.parse(selected[0].dataset.milestone);

            editMilestoneModal.querySelector('.milestone-id').value = milestone.id;
            editMilestoneModal.querySelector('.milestone-name').value = milestone.name;
            var container = editMilestoneModal.querySelector('.milestone-steps');
            container.innerHTML = '';
            if (milestone.steps.length) {
                milestone.steps.forEach(function (step) { addStepRow(container, step); });
            } else {
                addStepRow(container, { ratio: '1.00' });
            }
        });
    }

    // Restore the selection state after a back/forward navigation keeps checkboxes checked
    updateMilestoneSelection();
    </script>
    <?php endif; ?>
</body>
</html>
