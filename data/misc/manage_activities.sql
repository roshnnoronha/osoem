SELECT  taskname,taskdescription,notes,activityname, categoryname, subcategoryname, projectname,
plannedstart, plannedfinish, plannedhours
FROM Tasks
INNER JOIN Activities ON Activities.activityid = Tasks.activityid
INNER JOIN ActivityTaskList ON Tasks.activitytaskid = ActivityTaskList.activitytaskid
INNER JOIN Subcategories ON Activities.subcategoryid = Subcategories.subcategoryid
INNER JOIN Categories ON Subcategories.categoryid = Categories.categoryid
INNER JOIN Projects ON Categories.projectid = Projects.projectid 
WHERE taskid = 1;

