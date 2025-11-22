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
                    artefactname,
                    projectname,
                    Artefacts.artefactid,
                    Artefacts.artefacttypeid,
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
    
    public function getArtefactDataFields($artefacttypeid) {
        $sql = "SELECT artefactdatafieldid, fieldtitle FROM ArtefactDataFields WHERE artefacttypeid = ?";
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
    
    public function getArtefactsSummaryForActivity($activityid) {
        $sql = "SELECT
                    ArtefactTypes.artefactname AS Artefact,
                    Artefacts.artefacttitle AS Title,
                    COALESCE(MAX(MilestoneSteps.progressratio), 0) * 100 AS Progress,
                    CASE 
                        WHEN COALESCE(MAX(MilestoneSteps.progressratio), 0) < 1 THEN 'In-progress'
                        ELSE 'Complete'
                    END AS Status,
                    ArtefactToActivityLink.artefactlinkid
                FROM Artefacts
                INNER JOIN ArtefactTypes ON Artefacts.artefacttypeid = ArtefactTypes.artefacttypeid
                INNER JOIN ArtefactToActivityLink ON Artefacts.artefactid = ArtefactToActivityLink.artefactid
                LEFT JOIN MilestoneToArtefactLink ON ArtefactToActivityLink.artefactlinkid = MilestoneToArtefactLink.artefactlinkid
                LEFT JOIN MilestoneSteps ON MilestoneToArtefactLink.milestonestepid = MilestoneSteps.milestonestepid
                    AND NOT ISNULL(MilestoneToArtefactLink.completiondate)
                WHERE ArtefactToActivityLink.activityid = ?
                GROUP BY Artefacts.artefactid, ArtefactToActivityLink.artefactlinkid
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

    public function getMilestoneDetails($artefactlinkid) {
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
}
