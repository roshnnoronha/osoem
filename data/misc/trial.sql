SELECT 
    Milestones.milestonename,
    Artefacts.artefacttitle,
    Activities.activityname,
FROM MilestoneToArtefactLink 
INNER JOIN MilestoneSteps ON  MilestoneToArtefactLink.milestonestepid = MilestoneSteps.milestonestepid
INNER JOIN Milestones ON MilestoneSteps.milestoneid = Milestones.milestoneid
INNER JOIN ArtefactToActivityLink ON MilestoneToArtefactLink.artefactlinkid = ArtefactToActivityLink.artefactlinkid 
INNER JOIN Artefacts ON ArtefactToActivityLink.artefactid = Artefacts.artefactid
INNER JOIN Activities ON ArtefactToActivityLink.activityid = Activities.activityid
WHERE MilestoneToArtefactLink.artefactlinkid = 1
LIMIT 1;
