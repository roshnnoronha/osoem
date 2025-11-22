--query to get the task and activity details for a given task ID.
SELECT  taskname,taskdescription,tasknotes,activityname, categoryname, subcategoryname,
plannedhours, Activities.forecasthours,activitynotes, Projects.projectid,
concat(Employees.firstname,' ' ,Employees.lastname) AS activitymanager,
plannedstart, plannedfinish, forecaststart, forecastfinish, actualstart, actualfinish
FROM Tasks
INNER JOIN Activities ON Activities.activityid = Tasks.activityid
INNER JOIN ActivityTaskList ON Tasks.activitytaskid = ActivityTaskList.activitytaskid
INNER JOIN Subcategories ON Activities.subcategoryid = Subcategories.subcategoryid
INNER JOIN Categories ON Subcategories.categoryid = Categories.categoryid
INNER JOIN Projects ON Categories.projectid = Projects.projectid 
INNER JOIN Employees ON Activities.activitymanagerid = Employees.employeeid
WHERE taskid =?;

--get the current user of the task
SELECT concat(Employees.firstname,' ' ,Employees.lastname) AS username 
FROM Tasks
INNER JOIN Employees ON Employees.employeeid = Tasks.employeeid
WHERE taskid =?;

--get the consumed hours details
SELECT SUM(hours) AS consumedhours FROM Hours WHERE taskid = ?  AND employeeid = ?;
        
--get the artefact types for the project
SELECT artefacttypeid, artefactname FROM ArtefactTypes WHERE projectid = ?;

--get the details of all artefacts associated with the task
SELECT Artefacts.artefactid,artefactlinkid,artefactmark,artefacttitle FROM Artefacts
INNER JOIN ArtefactToActivityLink ON Artefacts.artefactid = ArtefactToActivityLink.artefactid
INNER JOIN Activities ON ArtefactToActivityLink.activityid = Activities.activityid
INNER JOIN Tasks ON Activities.activityid = Tasks.activityid
WHERE taskid = ? AND artefacttypeid = ?; 
                                                    
                    
