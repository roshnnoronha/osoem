SELECT concat(Employees.firstname,' ' ,Employees.lastname) AS username FROM Tasks
INNER JOIN Employees ON Employees.employeeid = Tasks.employeeid
WHERE taskid = 1;
