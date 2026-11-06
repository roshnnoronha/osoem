<?php
    require_once '../src/bootstrap.php';

    $artefactid = (int)($_GET['artefactid'] ?? 0);

    $artefactService = new ArtefactService();
    $activityService = new ActivityService();

    $data = $artefactid ? $artefactService->getArtefactDetailsById($artefactid) : null;

    if (!$data) {
        http_response_code(404);
        die('Artefact not found.');
    }

    $artefactType = $data['artefactname'];

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

    // Only the artefact owner and project Leads/Managers may edit the artefact data
    $canEditData = (int)$data['artefactownerid'] === (int)$eid
        || (new ProjectService())->isProjectLeadOrManager($data['projectid'], $eid);

    $dataFields = $artefactService->getArtefactDataFields($data['artefacttypeid']);

    $message = '';
    $messageType = '';
    $submittedValues = [];

    // Handle saving edits to the artefact data
    if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['save_data'])) {
        if (!Csrf::validate($_POST['csrf_token'] ?? '')) {
            http_response_code(403);
            die('Invalid or expired form submission (CSRF token mismatch). Please refresh the page and try again.');
        }

        if (!$canEditData) {
            http_response_code(403);
            die('Access denied: only the artefact owner, a project lead or the project manager can edit this data.');
        }

        $posted = $_POST['fields'] ?? [];
        $changes = [];
        $errors = [];

        foreach ($dataFields as $dataField) {
            $fieldid = (int)$dataField['artefactdatafieldid'];
            if (!$dataField['editable'] || !isset($posted[$fieldid]) || !is_string($posted[$fieldid])) {
                continue;
            }

            $value = trim($posted[$fieldid]);
            $submittedValues[$fieldid] = $value;
            $caption = $dataField['fieldtitle'];

            $maxLength = $dataField['maximumlength'] !== null ? min((int)$dataField['maximumlength'], 2000) : 2000;
            if (mb_strlen($value) > $maxLength) {
                $errors[] = "$caption must be at most $maxLength characters.";
                continue;
            }

            // valuetype 1 = number
            if ((int)$dataField['valuetype'] === 1 && $value !== '') {
                if (!is_numeric($value)) {
                    $errors[] = "$caption must be a number.";
                    continue;
                }
                if ($dataField['minimumvalue'] !== null && $value < $dataField['minimumvalue']) {
                    $errors[] = "$caption must be at least {$dataField['minimumvalue']}.";
                    continue;
                }
                if ($dataField['maximumvalue'] !== null && $value > $dataField['maximumvalue']) {
                    $errors[] = "$caption must be at most {$dataField['maximumvalue']}.";
                    continue;
                }
            }

            if ($value !== $artefactService->getArtefactFieldValue($artefactid, $fieldid)) {
                $changes[$fieldid] = $value;
            }
        }

        if ($errors) {
            $message = 'Changes not saved. ' . implode(' ', $errors);
            $messageType = 'warning';
        } elseif (empty($changes)) {
            $message = 'No changes to save.';
            $messageType = 'info';
        } elseif ($artefactService->saveArtefactFieldValues($artefactid, $changes)) {
            $message = 'Changes saved successfully!';
            $messageType = 'success';
            $submittedValues = [];
        } else {
            $message = 'Error saving changes. Please try again.';
            $messageType = 'danger';
        }
    }

    $fieldlist = [];
    $hasEditableFields = false;
    foreach ($dataFields as $dataField) {
        $fieldid = (int)$dataField['artefactdatafieldid'];
        $field = [];
        $field['id'] = $fieldid;
        $field['caption'] = $dataField['fieldtitle'];
        $field['savedvalue'] = $artefactService->getArtefactFieldValue($artefactid, $fieldid);
        // After a failed save, redisplay what the user typed so their edits aren't lost
        $field['value'] = $submittedValues[$fieldid] ?? $field['savedvalue'];
        $field['editable'] = $canEditData && $dataField['editable'];
        $field['numeric'] = (int)$dataField['valuetype'] === 1;
        $field['maximumlength'] = $dataField['maximumlength'];
        $fieldlist[] = $field;
        $hasEditableFields = $hasEditableFields || $field['editable'];
    }

    $artefactsUrl = 'artefacts.php?projectid=' . urlencode($data['projectid']);
    $artefactUrl = 'artefact.php?artefactid=' . urlencode($artefactid);

    $pageTitle = $data['artefacttitle'] . ' - Data';
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
                        <li class="breadcrumb-item"><a href="<?php echo $artefactsUrl; ?>">Artefacts</a></li>
                        <li class="breadcrumb-item"><a href="<?php echo $artefactUrl; ?>"><?php echo htmlspecialchars($data['artefacttitle']); ?></a></li>
                        <li class="breadcrumb-item active" aria-current="page">Data</li>
                    </ol>
                </nav>
                <div class="section-label"><?php echo htmlspecialchars($artefactType); ?></div>
                <h1 class="h3"><?php echo htmlspecialchars($data['artefacttitle']); ?></h1>
                <p class="page-subtitle"><?php echo htmlspecialchars($data['projectname']); ?></p>
            </div>
        </div>

        <?php if ($message): ?>
            <div class="alert alert-<?php echo $messageType; ?> alert-dismissible fade show" role="alert">
                <?php echo htmlspecialchars($message); ?>
                <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Close"></button>
            </div>
        <?php endif; ?>

        <div class="card">
            <div class="card-header">
                <h2 class="card-title">Data</h2>
            </div>
            <div class="card-body">
                <?php if (empty($fieldlist)): ?>
                    <div class="empty-state">
                        <i class="bi bi-inbox"></i>
                        This artefact type has no data fields.
                    </div>
                <?php else: ?>
                    <form method="POST" id="artefactDataForm">
                        <?php echo Csrf::field(); ?>
                        <?php foreach($fieldlist as $field): ?>
                            <?php
                                $fieldId = 'field-' . $field['id'];
                                $fieldCaption = htmlspecialchars($field['caption']);
                                $fieldValue = htmlspecialchars($field['value']);
                            ?>
                            <div class="mb-3">
                                <label class="form-label section-label mb-1" for="<?php echo $fieldId; ?>"><?php echo $fieldCaption; ?></label>
                                <?php if ($field['editable']): ?>
                                    <input type="text" class="form-control" id="<?php echo $fieldId; ?>"
                                           name="fields[<?php echo $field['id']; ?>]" value="<?php echo $fieldValue; ?>"
                                           data-saved-value="<?php echo htmlspecialchars($field['savedvalue']); ?>"
                                           <?php if ($field['numeric']): ?>inputmode="decimal"<?php endif; ?>
                                           maxlength="<?php echo $field['maximumlength'] !== null ? min((int)$field['maximumlength'], 2000) : 2000; ?>">
                                <?php else: ?>
                                    <input type="text" class="form-control-plaintext py-0" id="<?php echo $fieldId; ?>" value="<?php echo $fieldValue; ?>" readonly>
                                <?php endif; ?>
                            </div>
                        <?php endforeach; ?>
                        <?php if ($hasEditableFields): ?>
                            <div class="d-flex align-items-center gap-2">
                                <button type="submit" name="save_data" class="btn btn-primary" id="saveDataButton" disabled>Save</button>
                                <button type="button" class="btn btn-outline-secondary" id="discardDataButton" disabled>Discard changes</button>
                                <span class="text-muted small d-none" id="unsavedIndicator">Unsaved changes</span>
                            </div>
                        <?php endif; ?>
                    </form>
                <?php endif; ?>
            </div>
        </div>
    </main>

    <?php include 'includes/footer.php'; ?>

    <?php if ($hasEditableFields ?? false): ?>
    <script>
    (function () {
        var form = document.getElementById('artefactDataForm');
        var inputs = form.querySelectorAll('input[data-saved-value]');
        var saveButton = document.getElementById('saveDataButton');
        var discardButton = document.getElementById('discardDataButton');
        var indicator = document.getElementById('unsavedIndicator');
        var submitting = false;

        function isDirty() {
            return Array.prototype.some.call(inputs, function (input) {
                return input.value.trim() !== input.dataset.savedValue;
            });
        }

        function refresh() {
            var dirty = isDirty();
            saveButton.disabled = !dirty;
            discardButton.disabled = !dirty;
            indicator.classList.toggle('d-none', !dirty);
        }

        form.addEventListener('input', refresh);

        discardButton.addEventListener('click', function () {
            inputs.forEach(function (input) { input.value = input.dataset.savedValue; });
            refresh();
        });

        form.addEventListener('submit', function () { submitting = true; });

        window.addEventListener('beforeunload', function (event) {
            if (!submitting && isDirty()) {
                event.preventDefault();
                event.returnValue = '';
            }
        });

        refresh();
    })();
    </script>
    <?php endif; ?>
</body>
</html>
