<?php

require_once __DIR__ . '/../Database.php';

class ArtefactService {
    private $db;
    
    public function __construct() {
        $this->db = Database::getInstance();
    }
    
    public function getArtefactTypes($projectid) {
        $sql = "SELECT artefacttypeid, artefactname FROM ArtefactTypes WHERE projectid = ?";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $projectid);
        $stmt->execute();
        $result = $stmt->get_result();
        
        $artefacttypes = [];
        while ($row = $result->fetch_assoc()) {
            $artefacttypes[] = $row;
        }
        
        $stmt->close();
        return $artefacttypes;
    }
    
    public function getArtefactsByActivityAndType($activityid, $artefacttypeid) {
        $sql = "SELECT 
                    Artefacts.artefactid,
                    artefactlinkid,
                    artefacttitle 
                FROM Artefacts
                INNER JOIN ArtefactToActivityLink ON Artefacts.artefactid = ArtefactToActivityLink.artefactid
                WHERE ArtefactToActivityLink.activityid = ? AND Artefacts.artefacttypeid = ?";
        
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('ii', $activityid, $artefacttypeid);
        $stmt->execute();
        $result = $stmt->get_result();
        
        $artefacts = [];
        while ($row = $result->fetch_assoc()) {
            $artefacts[] = $row;
        }
        
        $stmt->close();
        return $artefacts;
    }
    
    public function getArtefactDetails($artefactlinkid) {
        $sql = "SELECT
                    artefacttitle,
                    Artefacts.artefactdescription,
                    artefactname,
                    projectname,
                    ArtefactTypes.projectid,
                    Artefacts.artefactid,
                    Artefacts.artefacttypeid,
                    Artefacts.artefactownerid,
                    ArtefactToActivityLink.activityid,
                    CONCAT(Employees.firstname, ' ', Employees.lastname) AS artefactowner
                FROM Artefacts
                INNER JOIN ArtefactTypes ON Artefacts.artefacttypeid = ArtefactTypes.artefacttypeid
                INNER JOIN Projects ON ArtefactTypes.projectid = Projects.projectid
                INNER JOIN ArtefactToActivityLink ON Artefacts.artefactid = ArtefactToActivityLink.artefactid
                LEFT JOIN Employees ON Artefacts.artefactownerid = Employees.employeeid
                WHERE artefactlinkid = ?";

        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $artefactlinkid);
        $stmt->execute();
        $result = $stmt->get_result();
        $data = $result->fetch_assoc();
        $stmt->close();

        return $data;
    }

    public function getArtefactDetailsById($artefactid) {
        $sql = "SELECT
                    artefacttitle,
                    Artefacts.artefactdescription,
                    artefactname,
                    projectname,
                    ArtefactTypes.projectid,
                    Artefacts.artefactid,
                    Artefacts.artefacttypeid,
                    Artefacts.artefactownerid,
                    CONCAT(Employees.firstname, ' ', Employees.lastname) AS artefactowner
                FROM Artefacts
                INNER JOIN ArtefactTypes ON Artefacts.artefacttypeid = ArtefactTypes.artefacttypeid
                INNER JOIN Projects ON ArtefactTypes.projectid = Projects.projectid
                LEFT JOIN Employees ON Artefacts.artefactownerid = Employees.employeeid
                WHERE Artefacts.artefactid = ?";

        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $artefactid);
        $stmt->execute();
        $result = $stmt->get_result();
        $data = $result->fetch_assoc();
        $stmt->close();

        return $data;
    }

    public function getArtefactDataFields($artefacttypeid) {
        $sql = "SELECT artefactdatafieldid, fieldtitle, valuetype, maximumlength, maximumvalue, minimumvalue, editable
                FROM ArtefactDataFields WHERE artefacttypeid = ?";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $artefacttypeid);
        $stmt->execute();
        $result = $stmt->get_result();
        
        $fields = [];
        while ($row = $result->fetch_assoc()) {
            $fields[] = $row;
        }
        
        $stmt->close();
        return $fields;
    }
    
    public function getArtefactFieldValue($artefactid, $artefactdatafieldid) {
        $sql = "SELECT value FROM ArtefactData WHERE artefactid = ? AND artefactdatafieldid = ?";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('ii', $artefactid, $artefactdatafieldid);
        $stmt->execute();
        $result = $stmt->get_result();
        $data = $result->fetch_assoc();
        $stmt->close();
        
        return $data['value'] ?? '';
    }

    // $values maps artefactdatafieldid => value. ArtefactData has no unique key on
    // (artefactid, artefactdatafieldid), so update the existing row or insert a new one.
    public function saveArtefactFieldValues($artefactid, $values) {
        $connection = $this->db->getConnection();
        $connection->begin_transaction();

        try {
            foreach ($values as $artefactdatafieldid => $value) {
                $sql = "SELECT artefactdataid FROM ArtefactData WHERE artefactid = ? AND artefactdatafieldid = ? LIMIT 1";
                $stmt = $this->db->prepare($sql);
                $stmt->bind_param('ii', $artefactid, $artefactdatafieldid);
                $stmt->execute();
                $existing = $stmt->get_result()->fetch_assoc();
                $stmt->close();

                if ($existing) {
                    $sql = "UPDATE ArtefactData SET value = ? WHERE artefactdataid = ?";
                    $stmt = $this->db->prepare($sql);
                    $stmt->bind_param('si', $value, $existing['artefactdataid']);
                } else {
                    $sql = "INSERT INTO ArtefactData (artefactid, artefactdatafieldid, value) VALUES (?, ?, ?)";
                    $stmt = $this->db->prepare($sql);
                    $stmt->bind_param('iis', $artefactid, $artefactdatafieldid, $value);
                }

                if (!$stmt->execute()) {
                    $stmt->close();
                    $connection->rollback();
                    return false;
                }
                $stmt->close();
            }

            $connection->commit();
            return true;
        } catch (mysqli_sql_exception $e) {
            $connection->rollback();
            return false;
        }
    }

    public function getArtefactsSummaryForActivity($activityid) {
        $sql = "SELECT
                    ArtefactTypes.artefactname AS artefact,
                    Artefacts.artefacttitle AS title,
                    COALESCE(MAX(MilestoneSteps.progressratio), 0) * 100 AS progress,
                    CASE
                        WHEN COALESCE(MAX(MilestoneSteps.progressratio), 0) < 1 THEN 'In-progress'
                        ELSE 'Complete'
                    END AS status,
                    GROUP_CONCAT(MilestoneSteps.milestonestepname
                        ORDER BY MilestoneSteps.progressratio SEPARATOR ', ') AS completedmilestones,
                    Artefacts.artefactid,
                    Artefacts.artefactownerid,
                    CONCAT(Employees.firstname, ' ', Employees.lastname) AS artefactowner,
                    ArtefactToActivityLink.artefactlinkid
                FROM Artefacts
                INNER JOIN ArtefactTypes ON Artefacts.artefacttypeid = ArtefactTypes.artefacttypeid
                LEFT JOIN Employees ON Artefacts.artefactownerid = Employees.employeeid
                INNER JOIN ArtefactToActivityLink ON Artefacts.artefactid = ArtefactToActivityLink.artefactid
                LEFT JOIN MilestoneToArtefactLink ON ArtefactToActivityLink.artefactlinkid = MilestoneToArtefactLink.artefactlinkid
                LEFT JOIN MilestoneSteps ON MilestoneToArtefactLink.milestonestepid = MilestoneSteps.milestonestepid
                    AND NOT ISNULL(MilestoneToArtefactLink.completiondate)
                WHERE ArtefactToActivityLink.activityid = ?
                GROUP BY Artefacts.artefactid, ArtefactToActivityLink.artefactlinkid, Employees.employeeid
                ORDER BY ArtefactTypes.artefactname, Artefacts.artefacttitle";
        
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $activityid);
        $stmt->execute();
        $result = $stmt->get_result();
        
        $artefacts = [];
        while ($row = $result->fetch_assoc()) {
            $artefacts[] = $row;
        }
        
        $stmt->close();
        return $artefacts;
    }

    // Overall progress is the ratio-weighted sum of each activity link's progress
    public function getArtefactsForProject($projectid) {
        $sql = "SELECT
                    ArtefactTypes.artefactname AS artefact,
                    Artefacts.artefacttypeid,
                    Artefacts.artefactid,
                    Artefacts.artefacttitle AS title,
                    Artefacts.artefactownerid,
                    CONCAT(Employees.firstname, ' ', Employees.lastname) AS artefactowner,
                    MIN(LinkProgress.artefactlinkid) AS artefactlinkid,
                    COUNT(LinkProgress.artefactlinkid) AS activity_count,
                    COALESCE(SUM(LinkProgress.ratio * LinkProgress.progress), 0) * 100 AS progress
                FROM Artefacts
                INNER JOIN ArtefactTypes ON Artefacts.artefacttypeid = ArtefactTypes.artefacttypeid
                LEFT JOIN Employees ON Artefacts.artefactownerid = Employees.employeeid
                LEFT JOIN (
                    SELECT
                        ArtefactToActivityLink.artefactlinkid,
                        ArtefactToActivityLink.artefactid,
                        ArtefactToActivityLink.ratio,
                        COALESCE(MAX(MilestoneSteps.progressratio), 0) AS progress
                    FROM ArtefactToActivityLink
                    LEFT JOIN MilestoneToArtefactLink ON ArtefactToActivityLink.artefactlinkid = MilestoneToArtefactLink.artefactlinkid
                    LEFT JOIN MilestoneSteps ON MilestoneToArtefactLink.milestonestepid = MilestoneSteps.milestonestepid
                        AND NOT ISNULL(MilestoneToArtefactLink.completiondate)
                    GROUP BY ArtefactToActivityLink.artefactlinkid, ArtefactToActivityLink.artefactid, ArtefactToActivityLink.ratio
                ) AS LinkProgress ON Artefacts.artefactid = LinkProgress.artefactid
                WHERE ArtefactTypes.projectid = ?
                GROUP BY Artefacts.artefactid, Artefacts.artefacttypeid, ArtefactTypes.artefactname, Artefacts.artefacttitle, Artefacts.artefactownerid, Employees.firstname, Employees.lastname
                ORDER BY ArtefactTypes.artefactname, Artefacts.artefacttitle";

        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $projectid);
        $stmt->execute();
        $result = $stmt->get_result();

        $artefacts = [];
        while ($row = $result->fetch_assoc()) {
            $row['status'] = $row['progress'] >= 100 ? 'Complete' : 'In-progress';
            $row['activities'] = [];
            $artefacts[(int)$row['artefactid']] = $row;
        }

        $stmt->close();

        foreach ($this->queryActivityLinks('ArtefactTypes.projectid = ?', $projectid) as $row) {
            $artefacts[(int)$row['artefactid']]['activities'][] = $row;
        }

        return array_values($artefacts);
    }

    // Activity links for one artefact, with each link's milestone progress
    public function getArtefactActivityLinks($artefactid) {
        return $this->queryActivityLinks('ArtefactToActivityLink.artefactid = ?', $artefactid);
    }

    // $where is a fixed condition with a single integer placeholder, never user input
    private function queryActivityLinks($where, $id) {
        $sql = "SELECT
                    ArtefactToActivityLink.artefactid,
                    ArtefactToActivityLink.artefactlinkid,
                    Activities.activityid,
                    Activities.activityname,
                    ArtefactToActivityLink.ratio,
                    COALESCE(MAX(CASE WHEN MilestoneToArtefactLink.completiondate IS NOT NULL
                        THEN MilestoneSteps.progressratio END), 0) * 100 AS progress,
                    GROUP_CONCAT(MilestoneSteps.milestonestepname
                        ORDER BY MilestoneSteps.progressratio SEPARATOR ', ') AS linkedmilestones,
                    GROUP_CONCAT(CASE WHEN MilestoneToArtefactLink.completiondate IS NOT NULL
                        THEN MilestoneSteps.milestonestepname END
                        ORDER BY MilestoneSteps.progressratio SEPARATOR ', ') AS completedmilestones
                FROM ArtefactToActivityLink
                INNER JOIN Activities ON ArtefactToActivityLink.activityid = Activities.activityid
                INNER JOIN Artefacts ON ArtefactToActivityLink.artefactid = Artefacts.artefactid
                INNER JOIN ArtefactTypes ON Artefacts.artefacttypeid = ArtefactTypes.artefacttypeid
                LEFT JOIN MilestoneToArtefactLink ON ArtefactToActivityLink.artefactlinkid = MilestoneToArtefactLink.artefactlinkid
                LEFT JOIN MilestoneSteps ON MilestoneToArtefactLink.milestonestepid = MilestoneSteps.milestonestepid
                WHERE $where
                GROUP BY ArtefactToActivityLink.artefactid, ArtefactToActivityLink.artefactlinkid, Activities.activityid, Activities.activityname, ArtefactToActivityLink.ratio
                ORDER BY Activities.activityname";

        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $id);
        $stmt->execute();
        $result = $stmt->get_result();

        $links = [];
        while ($row = $result->fetch_assoc()) {
            $links[] = $row;
        }

        $stmt->close();
        return $links;
    }

    public function getLinkedActivities($artefactid){
        $sql = "SELECT
                    ArtefactToActivityLink.artefactlinkid,
                    ArtefactToActivityLink.activityid,
                    ArtefactToActivityLink.ratio,
                    Activities.activityname
                FROM ArtefactToActivityLink
                INNER JOIN Activities ON ArtefactToActivityLink.activityid = Activities.activityid
                WHERE ArtefactToActivityLink.artefactid = ?;";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $artefactid);
        $stmt->execute();
        $result = $stmt->get_result();

        $linkedActivities = [];
        while ($row = $result->fetch_assoc()) {
            $linkedActivities [] = $row;
        }
        $stmt->close();
        return $linkedActivities;
    }

    public function getArtefactProgressByActivity($artefactid) {
        $sql = "SELECT
                    Activities.activityname,
                    ArtefactToActivityLink.ratio,
                    ArtefactToActivityLink.artefactlinkid,
                    COALESCE(MAX(MilestoneSteps.progressratio), 0) * 100 AS progress
                FROM ArtefactToActivityLink
                INNER JOIN Activities ON ArtefactToActivityLink.activityid = Activities.activityid
                LEFT JOIN MilestoneToArtefactLink ON ArtefactToActivityLink.artefactlinkid = MilestoneToArtefactLink.artefactlinkid
                LEFT JOIN MilestoneSteps ON MilestoneToArtefactLink.milestonestepid = MilestoneSteps.milestonestepid
                    AND NOT ISNULL(MilestoneToArtefactLink.completiondate)
                WHERE ArtefactToActivityLink.artefactid = ?
                GROUP BY ArtefactToActivityLink.artefactlinkid, Activities.activityname, ArtefactToActivityLink.ratio
                ORDER BY Activities.activityname;";

        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $artefactid);
        $stmt->execute();
        $result = $stmt->get_result();

        $progressData = [];
        while ($row = $result->fetch_assoc()) {
            $progressData[] = $row;
        }

        $stmt->close();
        return $progressData;
    }

    // Adds an artefact type and its data fields in one transaction. Each entry of $fields has keys
    // fieldtitle, valuetype (0=text, 1=number), maximumlength, minimumvalue, maximumvalue (int or null) and editable (bool).
    public function addArtefactType($projectid, $artefactname, $artefactdescription = null, array $fields = []) {
        $connection = $this->db->getConnection();
        $connection->begin_transaction();

        try {
            $sql = "INSERT INTO ArtefactTypes (projectid, artefactname, artefactdescription) VALUES (?, ?, ?)";
            $stmt = $this->db->prepare($sql);
            $stmt->bind_param('iss', $projectid, $artefactname, $artefactdescription);
            $stmt->execute();
            $artefacttypeid = $stmt->insert_id;
            $stmt->close();

            $sql = "INSERT INTO ArtefactDataFields (artefacttypeid, fieldtitle, valuetype, maximumlength, maximumvalue, minimumvalue, editable)
                    VALUES (?, ?, ?, ?, ?, ?, ?)";
            foreach ($fields as $field) {
                $editable = $field['editable'] ? 1 : 0;
                $stmt = $this->db->prepare($sql);
                $stmt->bind_param('isiiiii', $artefacttypeid, $field['fieldtitle'], $field['valuetype'],
                    $field['maximumlength'], $field['maximumvalue'], $field['minimumvalue'], $editable);
                $stmt->execute();
                $stmt->close();
            }

            $connection->commit();
            return true;
        } catch (mysqli_sql_exception $e) {
            $connection->rollback();
            return false;
        }
    }

    public function addArtefact($artefacttypeid, $artefacttitle, $artefactdescription, $artefactownerid, $activityid, $ratio) {
        $sqlArtefact = "INSERT INTO Artefacts (artefacttypeid, artefactownerid, artefacttitle, artefactdescription) VALUES (?, ?, ?, ?)";
        $stmtArtefact = $this->db->prepare($sqlArtefact);
        $stmtArtefact->bind_param('iiss', $artefacttypeid, $artefactownerid, $artefacttitle, $artefactdescription);
        if (!$stmtArtefact->execute()) {
            $stmtArtefact->close();
            return false;
        }
        $artefactid = $stmtArtefact->insert_id;
        $stmtArtefact->close();

        // Linking to an activity is optional
        if ($activityid === null) {
            return true;
        }

        $sqlLink = "INSERT INTO ArtefactToActivityLink (activityid, artefactid, ratio) VALUES (?, ?, ?)";
        $stmtLink = $this->db->prepare($sqlLink);
        $stmtLink->bind_param('iid', $activityid, $artefactid, $ratio);
        $result = $stmtLink->execute();
        $stmtLink->close();

        return $result;
    }

    // Links an existing artefact to an activity. The sum of an artefact's link ratios must not exceed 1.
    public function linkArtefactToActivity($artefactid, $activityid, $ratio) {
        $connection = $this->db->getConnection();
        $connection->begin_transaction();

        try {
            // Lock the artefact's existing links so concurrent links can't push the ratio sum over 1
            $sql = "SELECT activityid, ratio FROM ArtefactToActivityLink WHERE artefactid = ? FOR UPDATE";
            $stmt = $this->db->prepare($sql);
            $stmt->bind_param('i', $artefactid);
            $stmt->execute();
            $result = $stmt->get_result();

            $allocated = 0;
            while ($row = $result->fetch_assoc()) {
                if ($row['activityid'] == $activityid) {
                    $stmt->close();
                    $connection->rollback();
                    return ['success' => false, 'message' => 'This artefact is already linked to this activity.'];
                }
                $allocated += (float)$row['ratio'];
            }
            $stmt->close();

            if (round($allocated + $ratio, 2) > 1) {
                $connection->rollback();
                $remaining = number_format(max(0, 1 - $allocated), 2);
                return ['success' => false, 'message' => "The ratio exceeds the artefact's unallocated ratio ($remaining)."];
            }

            $sql = "INSERT INTO ArtefactToActivityLink (activityid, artefactid, ratio) VALUES (?, ?, ?)";
            $stmt = $this->db->prepare($sql);
            $stmt->bind_param('iid', $activityid, $artefactid, $ratio);
            $stmt->execute();
            $stmt->close();

            $connection->commit();
            return ['success' => true, 'message' => 'Artefact linked successfully!'];
        } catch (mysqli_sql_exception $e) {
            $connection->rollback();
            return ['success' => false, 'message' => 'Error linking artefact. Please try again.'];
        }
    }

    // Updates the ratios of an artefact's activity links; $ratios maps artefactlinkid => ratio and must cover every link
    public function updateLinkRatios($artefactid, array $ratios) {
        $connection = $this->db->getConnection();
        $connection->begin_transaction();

        try {
            // Lock the artefact's links so concurrent changes can't push the ratio sum over 1
            $sql = "SELECT artefactlinkid FROM ArtefactToActivityLink WHERE artefactid = ? FOR UPDATE";
            $stmt = $this->db->prepare($sql);
            $stmt->bind_param('i', $artefactid);
            $stmt->execute();
            $result = $stmt->get_result();

            $linkids = [];
            while ($row = $result->fetch_assoc()) {
                $linkids[] = (int)$row['artefactlinkid'];
            }
            $stmt->close();

            $submitted = array_map('intval', array_keys($ratios));
            sort($linkids);
            sort($submitted);
            if ($linkids !== $submitted) {
                $connection->rollback();
                return ['success' => false, 'message' => 'The linked activities have changed. Please refresh the page and try again.'];
            }

            if (round(array_sum($ratios), 2) > 1) {
                $connection->rollback();
                return ['success' => false, 'message' => 'The sum of the link ratios must not exceed 1.00.'];
            }

            $sql = "UPDATE ArtefactToActivityLink SET ratio = ? WHERE artefactlinkid = ? AND artefactid = ?";
            $stmt = $this->db->prepare($sql);
            foreach ($ratios as $linkid => $ratio) {
                $stmt->bind_param('dii', $ratio, $linkid, $artefactid);
                $stmt->execute();
            }
            $stmt->close();

            $connection->commit();
            return ['success' => true, 'message' => 'Link ratios updated successfully!'];
        } catch (mysqli_sql_exception $e) {
            $connection->rollback();
            return ['success' => false, 'message' => 'Error updating link ratios. Please try again.'];
        }
    }

    // Removes an artefact's link to an activity, along with the milestone steps recorded against that link
    public function unlinkArtefactFromActivity($artefactlinkid) {
        $connection = $this->db->getConnection();
        $connection->begin_transaction();

        try {
            $sql = "DELETE FROM MilestoneToArtefactLink WHERE artefactlinkid = ?";
            $stmt = $this->db->prepare($sql);
            $stmt->bind_param('i', $artefactlinkid);
            $stmt->execute();
            $stmt->close();

            $sql = "DELETE FROM ArtefactToActivityLink WHERE artefactlinkid = ?";
            $stmt = $this->db->prepare($sql);
            $stmt->bind_param('i', $artefactlinkid);
            $stmt->execute();
            $stmt->close();

            $connection->commit();
            return true;
        } catch (mysqli_sql_exception $e) {
            $connection->rollback();
            return false;
        }
    }

    // Permanently deletes artefacts along with their data values, activity links and the
    // milestone progress recorded against those links. All or nothing.
    public function deleteArtefacts(array $artefactids) {
        $artefactids = array_values(array_unique(array_map('intval', $artefactids)));
        if (empty($artefactids)) {
            return false;
        }

        $placeholders = implode(',', array_fill(0, count($artefactids), '?'));
        $types = str_repeat('i', count($artefactids));
        $statements = [
            "DELETE MilestoneToArtefactLink FROM MilestoneToArtefactLink
                INNER JOIN ArtefactToActivityLink ON MilestoneToArtefactLink.artefactlinkid = ArtefactToActivityLink.artefactlinkid
                WHERE ArtefactToActivityLink.artefactid IN ($placeholders)",
            "DELETE FROM ArtefactToActivityLink WHERE artefactid IN ($placeholders)",
            "DELETE FROM ArtefactData WHERE artefactid IN ($placeholders)",
            "DELETE FROM Artefacts WHERE artefactid IN ($placeholders)",
        ];

        $connection = $this->db->getConnection();
        $connection->begin_transaction();

        try {
            foreach ($statements as $sql) {
                $stmt = $this->db->prepare($sql);
                $stmt->bind_param($types, ...$artefactids);
                $stmt->execute();
                $stmt->close();
            }

            $connection->commit();
            return true;
        } catch (mysqli_sql_exception $e) {
            $connection->rollback();
            return false;
        }
    }

    public function delegateArtefact($artefactid, $newownerid) {
        $sql = "UPDATE Artefacts SET artefactownerid = ? WHERE artefactid = ?";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('ii', $newownerid, $artefactid);
        $result = $stmt->execute();
        $stmt->close();
        return $result;
    }

    public function updateArtefact($artefactid, $artefacttitle, $artefactdescription) {
        $sql = "UPDATE Artefacts SET artefacttitle = ?, artefactdescription = ? WHERE artefactid = ?";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('ssi', $artefacttitle, $artefactdescription, $artefactid);
        $result = $stmt->execute();
        $stmt->close();
        return $result;
    }
}
