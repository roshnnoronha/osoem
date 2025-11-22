<?php

require_once __DIR__ . '/../Database.php';

class MilestoneService {
    private $db;

    public function __construct() {
        $this->db = Database::getInstance();
    }

    public function getMilestoneSteps($artefactlinkid) {
        $sql = "SELECT
                    MilestoneSteps.milestonestepname,
                    CASE
                        WHEN ISNULL(MilestoneToArtefactLink.completiondate) THEN 'In progress'
                        ELSE 'Complete'
                    END AS Status,
                    CASE
                        WHEN ISNULL(MilestoneToArtefactLink.completedbyid) THEN '-'
                        ELSE CONCAT (Employees.firstname, ' ' , Employees.lastname)
                    END AS Completed_By
                FROM MilestoneSteps
                INNER JOIN Milestones ON MilestoneSteps.milestoneid = Milestones.milestoneid
                INNER JOIN MilestoneToArtefactLink ON MilestoneToArtefactLink.milestonestepid = MilestoneSteps.milestonestepid
                INNER JOIN ArtefactToActivityLink ON ArtefactToActivityLink.artefactlinkid = MilestoneToArtefactLink.artefactlinkid
                LEFT JOIN Employees ON MilestoneToArtefactLink.completedbyid = Employees.employeeid
                WHERE MilestoneToArtefactLink.artefactlinkid = ?;";

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
                    Activities.activityname
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

}
