--get the details of artefact
SELECT artefactmark, artefacttitle, artefactname, projectname,Artefacts.artefactid, Artefacts.artefacttypeid FROM Artefacts
INNER JOIN ArtefactTypes ON Artefacts.artefacttypeid = ArtefactTypes.artefacttypeid
INNER JOIN Projects ON ArtefactTypes.projectid = Projects.projectid
INNER JOIN ArtefactToActivityLink ON Artefacts.artefactid = ArtefactToActivityLink.artefactid
WHERE artefactlinkid= ?;

--get the list of fields for the artefact
SELECT artefactdatafieldid, fieldtitle FROM ArtefactDataFields WHERE artefacttypeid = ?;

--get field values
SELECT value FROM ArtefactData WHERE artefactid = ? AND artefactdatafieldid = ?;

--get the milestone details
SELECT milestonename, milestonetypename, progressratio,
CASE
    WHEN ISNULL(MilestoneToArtefactLink.completiondate) AND ISNULL(MilestoneToArtefactLink.approveddate) THEN 'In progress'
    WHEN ISNULL(MilestoneToArtefactLink.approveddate) THEN 'Pending approval'
    ELSE 'Complete'
END AS status
FROM Milestones 
INNER JOIN MilestoneTypes ON MilestoneTypes.milestonetypeid = Milestones.milestonetypeid 
INNER JOIN MilestoneToArtefactLink ON MilestoneToArtefactLink.milestoneid = Milestones.milestoneid 
INNER JOIN ArtefactToActivityLink ON ArtefactToActivityLink.artefactlinkid = MilestoneToArtefactLink.artefactlinkid
WHERE MilestoneToArtefactLink.artefactlinkid = ?;
