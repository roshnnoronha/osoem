<?php

require_once __DIR__ . '/../Database.php';

class MilestoneService {
    private $db;

    public function __construct() {
        $this->db = Database::getInstance();
    }

    public function getMilestoneSteps($artefactlinkid) {
        $sql = "SELECT
                    MilestoneToArtefactLink.milestonelinkid,
                    MilestoneSteps.milestonestepname,
                    MilestoneSteps.progressratio,
                    MilestoneToArtefactLink.completiondate,
                    MilestoneToArtefactLink.planneddate AS planned_date,
                    MilestoneToArtefactLink.forecastdate AS forecast_date,
                    CASE
                        WHEN ISNULL(MilestoneToArtefactLink.completiondate) THEN 'In progress'
                        ELSE 'Complete'
                    END AS status,
                    CASE
                        WHEN ISNULL(MilestoneToArtefactLink.completedbyid) THEN '-'
                        ELSE CONCAT (Employees.firstname, ' ' , Employees.lastname)
                    END AS completed_by
                FROM MilestoneSteps
                INNER JOIN Milestones ON MilestoneSteps.milestoneid = Milestones.milestoneid
                INNER JOIN MilestoneToArtefactLink ON MilestoneToArtefactLink.milestonestepid = MilestoneSteps.milestonestepid
                INNER JOIN ArtefactToActivityLink ON ArtefactToActivityLink.artefactlinkid = MilestoneToArtefactLink.artefactlinkid
                LEFT JOIN Employees ON MilestoneToArtefactLink.completedbyid = Employees.employeeid
                WHERE MilestoneToArtefactLink.artefactlinkid = ?
                ORDER BY MilestoneSteps.progressratio;";

        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $artefactlinkid);
        $stmt->execute();
        $result = $stmt->get_result();

        $milestones = [];
        while ($row = $result->fetch_assoc()) {
            $milestones[] = $row;
        }

        $stmt->close();
        return $milestones;
    }

    public function getMilestoneDetails($artefactlinkid) {
        $sql = "SELECT
                    Milestones.milestonename,
                    Artefacts.artefacttitle,
                    Artefacts.artefactid,
                    Activities.activityname,
                    Activities.activityid
                FROM MilestoneToArtefactLink
                INNER JOIN MilestoneSteps ON  MilestoneToArtefactLink.milestonestepid = MilestoneSteps.milestonestepid
                INNER JOIN Milestones ON MilestoneSteps.milestoneid = Milestones.milestoneid
                INNER JOIN ArtefactToActivityLink ON MilestoneToArtefactLink.artefactlinkid = ArtefactToActivityLink.artefactlinkid 
                INNER JOIN Artefacts ON ArtefactToActivityLink.artefactid = Artefacts.artefactid
                INNER JOIN Activities ON ArtefactToActivityLink.activityid = Activities.activityid
                WHERE MilestoneToArtefactLink.artefactlinkid = ?
                LIMIT 1; ";

        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $artefactlinkid);
        $stmt->execute();
        $result = $stmt->get_result();
        $data = $result->fetch_assoc();
        $stmt->close();

        return $data;
    }

    // Returns the project's milestones keyed by milestoneid, each with its steps in progress order.
    // 'artefactcount' is the number of artefact links tracking the milestone.
    public function getMilestonesForProject($projectid) {
        $sql = "SELECT
                    Milestones.milestoneid,
                    Milestones.milestonename,
                    MilestoneSteps.milestonestepid,
                    MilestoneSteps.milestonestepname,
                    MilestoneSteps.progressratio,
                    (SELECT COUNT(DISTINCT MilestoneToArtefactLink.artefactlinkid)
                     FROM MilestoneToArtefactLink
                     INNER JOIN MilestoneSteps AS UsedSteps ON MilestoneToArtefactLink.milestonestepid = UsedSteps.milestonestepid
                     WHERE UsedSteps.milestoneid = Milestones.milestoneid) AS artefactcount
                FROM Milestones
                LEFT JOIN MilestoneSteps ON Milestones.milestoneid = MilestoneSteps.milestoneid
                WHERE Milestones.projectid = ?
                ORDER BY Milestones.milestonename, MilestoneSteps.progressratio";

        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $projectid);
        $stmt->execute();
        $result = $stmt->get_result();

        $milestones = [];
        while ($row = $result->fetch_assoc()) {
            $milestoneid = (int)$row['milestoneid'];
            if (!isset($milestones[$milestoneid])) {
                $milestones[$milestoneid] = [
                    'milestoneid' => $milestoneid,
                    'milestonename' => $row['milestonename'],
                    'artefactcount' => (int)$row['artefactcount'],
                    'steps' => [],
                ];
            }
            if ($row['milestonestepid'] !== null) {
                $milestones[$milestoneid]['steps'][] = $row;
            }
        }

        $stmt->close();
        return $milestones;
    }

    // Links every step of $steps to the artefact link. $planneddates maps milestonestepid => date (or null);
    // the forecast date starts out equal to the planned date.
    public function associateMilestone($artefactlinkid, array $steps, array $planneddates) {
        $conn = $this->db->getConnection();

        $conn->begin_transaction();
        try {
            $sql = "INSERT INTO MilestoneToArtefactLink (milestonestepid, artefactlinkid, planneddate, forecastdate) VALUES (?, ?, ?, ?)";
            foreach ($steps as $step) {
                $planneddate = $planneddates[$step['milestonestepid']] ?? null;
                $stmt = $this->db->prepare($sql);
                $stmt->bind_param('iiss', $step['milestonestepid'], $artefactlinkid, $planneddate, $planneddate);
                $stmt->execute();
                $stmt->close();
            }

            $conn->commit();
            return true;
        } catch (Exception $e) {
            $conn->rollback();
            return false;
        }
    }

    // Marks the given milestone links complete (if not already) and clears completion on the rest.
    // Only links belonging to $artefactlinkid are touched.
    public function setCompletedMilestoneSteps($artefactlinkid, array $completedlinkids, $completedbyid, $completiondate) {
        $completedlinkids = array_values(array_map('intval', $completedlinkids));
        $conn = $this->db->getConnection();

        $conn->begin_transaction();
        try {
            if ($completedlinkids) {
                $placeholders = implode(',', array_fill(0, count($completedlinkids), '?'));
                $sql = "UPDATE MilestoneToArtefactLink
                        SET completiondate = ?, completedbyid = ?
                        WHERE artefactlinkid = ? AND completiondate IS NULL
                          AND milestonelinkid IN ($placeholders)";
                $stmt = $this->db->prepare($sql);
                $stmt->bind_param('sii' . str_repeat('i', count($completedlinkids)),
                    $completiondate, $completedbyid, $artefactlinkid, ...$completedlinkids);
                $stmt->execute();
                $stmt->close();

                $sql = "UPDATE MilestoneToArtefactLink
                        SET completiondate = NULL, completedbyid = NULL
                        WHERE artefactlinkid = ? AND milestonelinkid NOT IN ($placeholders)";
                $stmt = $this->db->prepare($sql);
                $stmt->bind_param('i' . str_repeat('i', count($completedlinkids)), $artefactlinkid, ...$completedlinkids);
            } else {
                $sql = "UPDATE MilestoneToArtefactLink
                        SET completiondate = NULL, completedbyid = NULL
                        WHERE artefactlinkid = ?";
                $stmt = $this->db->prepare($sql);
                $stmt->bind_param('i', $artefactlinkid);
            }
            $stmt->execute();
            $stmt->close();

            $conn->commit();
            return true;
        } catch (Exception $e) {
            $conn->rollback();
            return false;
        }
    }

    // Returns the milestone row (milestoneid, projectid, milestonename), or null if it does not exist
    public function getMilestone($milestoneid) {
        $stmt = $this->db->prepare("SELECT milestoneid, projectid, milestonename FROM Milestones WHERE milestoneid = ?");
        $stmt->bind_param('i', $milestoneid);
        $stmt->execute();
        $row = $stmt->get_result()->fetch_assoc();
        $stmt->close();

        return $row ?: null;
    }

    // Creates a milestone with its steps. $steps is a list of ['name' => string, 'ratio' => float].
    // Returns the new milestoneid, or false on failure.
    public function createMilestone($projectid, $milestonename, array $steps) {
        $conn = $this->db->getConnection();

        $conn->begin_transaction();
        try {
            $milestoneid = $this->execute(
                "INSERT INTO Milestones (projectid, milestonename) VALUES (?, ?)",
                'is', $projectid, $milestonename
            );
            foreach ($steps as $step) {
                $this->execute(
                    "INSERT INTO MilestoneSteps (milestoneid, milestonestepname, progressratio) VALUES (?, ?, ?)",
                    'isd', $milestoneid, $step['name'], $step['ratio']
                );
            }

            $conn->commit();
            return $milestoneid;
        } catch (Exception $e) {
            $conn->rollback();
            return false;
        }
    }

    // Renames a milestone and updates its steps. $steps is a list of
    // ['milestonestepid' => int|null, 'name' => string, 'ratio' => float]: steps with an id are updated
    // and steps without one are added. Every existing step must be in the list (steps cannot be
    // removed). New steps are added to every artefact link already
    // tracking the milestone so their progress stays complete.
    public function updateMilestone($milestoneid, $milestonename, array $steps) {
        $conn = $this->db->getConnection();

        $conn->begin_transaction();
        try {
            $this->execute("UPDATE Milestones SET milestonename = ? WHERE milestoneid = ?", 'si', $milestonename, $milestoneid);

            $existingids = array_map('intval', array_column($this->fetchAll(
                "SELECT milestonestepid FROM MilestoneSteps WHERE milestoneid = ?", 'i', $milestoneid
            ), 'milestonestepid'));
            $trackinglinkids = array_map('intval', array_column($this->fetchAll(
                "SELECT DISTINCT MilestoneToArtefactLink.artefactlinkid
                 FROM MilestoneToArtefactLink
                 INNER JOIN MilestoneSteps ON MilestoneToArtefactLink.milestonestepid = MilestoneSteps.milestonestepid
                 WHERE MilestoneSteps.milestoneid = ?", 'i', $milestoneid
            ), 'artefactlinkid'));

            $keptids = [];
            foreach ($steps as $step) {
                $stepid = $step['milestonestepid'] ?? null;
                if ($stepid !== null) {
                    if (!in_array((int)$stepid, $existingids, true)) {
                        throw new Exception('Step does not belong to this milestone.');
                    }
                    $keptids[] = (int)$stepid;
                    $this->execute(
                        "UPDATE MilestoneSteps SET milestonestepname = ?, progressratio = ? WHERE milestonestepid = ?",
                        'sdi', $step['name'], $step['ratio'], $stepid
                    );
                } else {
                    $newstepid = $this->execute(
                        "INSERT INTO MilestoneSteps (milestoneid, milestonestepname, progressratio) VALUES (?, ?, ?)",
                        'isd', $milestoneid, $step['name'], $step['ratio']
                    );
                    foreach ($trackinglinkids as $artefactlinkid) {
                        $this->execute(
                            "INSERT INTO MilestoneToArtefactLink (milestonestepid, artefactlinkid) VALUES (?, ?)",
                            'ii', $newstepid, $artefactlinkid
                        );
                    }
                }
            }

            if (array_diff($existingids, $keptids)) {
                throw new Exception('Milestone steps cannot be removed.');
            }

            $conn->commit();
            return true;
        } catch (Exception $e) {
            $conn->rollback();
            return false;
        }
    }

    // Runs a write statement, throwing on failure. Returns the insert id (0 for non-inserts).
    private function execute($sql, $types, ...$params) {
        $stmt = $this->db->prepare($sql);
        if (!$stmt) {
            throw new Exception('Failed to prepare statement.');
        }
        $stmt->bind_param($types, ...$params);
        if (!$stmt->execute()) {
            $stmt->close();
            throw new Exception('Failed to execute statement.');
        }
        $insertid = $stmt->insert_id;
        $stmt->close();
        return $insertid;
    }

    private function fetchAll($sql, $types, ...$params) {
        $stmt = $this->db->prepare($sql);
        if (!$stmt) {
            throw new Exception('Failed to prepare statement.');
        }
        $stmt->bind_param($types, ...$params);
        $stmt->execute();
        $rows = $stmt->get_result()->fetch_all(MYSQLI_ASSOC);
        $stmt->close();
        return $rows;
    }
}
