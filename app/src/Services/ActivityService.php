<?php

require_once __DIR__ . '/../Database.php';

class ActivityService {
    private $db;
    
    public function __construct() {
        $this->db = Database::getInstance();
    }
    
    public function getUserTasks($employeeid) {
        $sql = "SELECT 
                    ActivityTasks.taskid,
                    Activities.activityid,
                    Activities.activityname,
                    ActivityTasks.taskname,
                    Projects.projectname 
                FROM ActivityTasks
                INNER JOIN ActivityTaskAssignments ON ActivityTasks.taskid = ActivityTaskAssignments.taskid
                INNER JOIN Activities ON Activities.activityid = ActivityTasks.activityid
                INNER JOIN ActivitySubcategories ON Activities.subcategoryid = ActivitySubcategories.subcategoryid
                INNER JOIN ActivityCategories ON ActivitySubcategories.categoryid = ActivityCategories.categoryid
                INNER JOIN Projects ON ActivityCategories.projectid = Projects.projectid
                WHERE ActivityTaskAssignments.userid = ?";
        
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $employeeid);
        $stmt->execute();
        $result = $stmt->get_result();
        
        $data = [];
        while ($row = $result->fetch_assoc()) {
            $data[] = $row;
        }
        
        $stmt->close();
        return $data;
    }
    
    public function getActivityDetails($activityid) {
        $sql = "SELECT  
                    Activities.activityname AS Activity_Name,
                    Activities.activitydescription AS Description, 
                    Projects.projectno AS Project_Number,
                    Projects.projectname AS Project_Name,
                    ActivityCategories.categoryname AS Category,
                    ActivitySubcategories.subcategoryname AS Subcategory,
                    CONCAT(Employees.firstname,' ' ,Employees.lastname) AS Activity_Manager,
                    Activities.plannedstart AS Planned_Start,
                    Activities.plannedfinish AS Planned_Finish,
                    Activities.forecaststart AS Forecast_Start,
                    Activities.forecastfinish AS Forecast_Finish,
                    Activities.actualstart AS Actual_Start,
                    Activities.actualfinish AS Actual_Finish,
                    Activities.plannedhours AS Planned_Hours,
                    Activities.forecasthours AS Forecast_Hours,
                    Projects.projectid
                FROM Activities
                INNER JOIN ActivitySubcategories ON Activities.subcategoryid = ActivitySubcategories.subcategoryid
                INNER JOIN ActivityCategories ON ActivitySubcategories.categoryid = ActivityCategories.categoryid
                INNER JOIN Projects ON ActivityCategories.projectid = Projects.projectid 
                INNER JOIN Employees ON Activities.activitymanagerid = Employees.employeeid
                WHERE Activities.activityid = ?";
        
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $activityid);
        $stmt->execute();
        $result = $stmt->get_result();
        $data = $result->fetch_assoc();
        $stmt->close();
        
        return $data;
    }
    
    public function getConsumedHours($activityid) {
        $sql = "SELECT 
                    SUM(Hours.hours) AS Consumed_Hours 
                FROM Hours 
                INNER JOIN ActivityTaskAssignments ON Hours.assignmentid = ActivityTaskAssignments.assignmentid
                INNER JOIN ActivityTasks ON ActivityTaskAssignments.taskid = ActivityTasks.taskid
                INNER JOIN Activities ON ActivityTasks.activityid = Activities.activityid
                WHERE Activities.activityid = ?";
        
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $activityid);
        $stmt->execute();
        $result = $stmt->get_result();
        $data = $result->fetch_assoc();
        $stmt->close();
        
        return $data['Consumed_Hours'] ?? 0;
    }
    
    public function getActivityNotes($activityid) {
        $sql = "SELECT
                   ActivityNotes.noteid AS Note_ID,
                   ActivityNotes.userid AS User_ID,
                   note AS Note,
                   concat(Employees.firstname,' ' ,Employees.lastname) AS Note_By,
                   notedate AS Note_Date
                FROM ActivityNotes
                INNER JOIN Activities ON ActivityNotes.activityid = Activities.activityid
                INNER JOIN Employees ON ActivityNotes.userid = Employees.employeeid
                WHERE Activities.activityid = ?
                ORDER BY ActivityNotes.notedate DESC, ActivityNotes.noteid DESC";
        
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $activityid);
        $stmt->execute();
        $result = $stmt->get_result();
        
        $notes = [];
        while ($row = $result->fetch_assoc()) {
            $notes[] = $row;
        }
        
        $stmt->close();
        return $notes;
    }
    
    public function getActivityTasksForUser($activityid, $userid) {
        $sql = "SELECT
                    ActivityTasks.taskid,
                    ActivityTaskAssignments.assignmentid,
                    ActivityTasks.taskname AS Task_Name,
                    concat(Employees.firstname,' ' ,Employees.lastname) AS Assigned_To,
                    ActivityTaskAssignments.assigneddate AS Assigned_Date,
                    ActivityTaskAssignments.closedate AS Closed_Date,
                    COALESCE(SUM(Hours.hours), 0) AS Booked_Hours,
                    CASE
                        WHEN ActivityTaskAssignments.closedate IS NULL THEN 'Open'
                        ELSE 'Closed'
                    END AS Status
                FROM ActivityTasks
                INNER JOIN ActivityTaskAssignments ON ActivityTasks.taskid = ActivityTaskAssignments.taskid
                INNER JOIN Employees ON ActivityTaskAssignments.userid = Employees.employeeid
                LEFT JOIN Hours ON ActivityTaskAssignments.assignmentid = Hours.assignmentid
                WHERE ActivityTasks.activityid = ? AND ActivityTaskAssignments.userid = ? AND ActivityTaskAssignments.closedate IS NULL
                GROUP BY ActivityTasks.taskid, ActivityTaskAssignments.assignmentid
                ORDER BY ActivityTaskAssignments.assigneddate";
        
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('ii', $activityid, $userid);
        $stmt->execute();
        $result = $stmt->get_result();
        
        $tasks = [];
        while ($row = $result->fetch_assoc()) {
            // Add hierarchical task name
            $row['Task_Hierarchy'] = $this->getTaskHierarchy($row['taskid']);
            $tasks[] = $row;
        }
        
        $stmt->close();
        return $tasks;
    }

    private function getTaskHierarchy($taskid) {
        $hierarchy = [];
        $currentTaskId = $taskid;
        
        while ($currentTaskId) {
            $sql = "SELECT taskname, parenttaskid FROM ActivityTasks WHERE taskid = ?";
            $stmt = $this->db->prepare($sql);
            $stmt->bind_param('i', $currentTaskId);
            $stmt->execute();
            $result = $stmt->get_result();
            $task = $result->fetch_assoc();
            $stmt->close();
            
            if ($task) {
                array_unshift($hierarchy, $task['taskname']);
                $currentTaskId = $task['parenttaskid'];
            } else {
                break;
            }
        }
        
        return implode('/', $hierarchy);
    }

    public function getUserActivities($employeeid) {
        $sql = "SELECT DISTINCT
                    Activities.activityid,
                    Activities.activityname,
                    Activities.activitydescription,
                    Projects.projectname,
                    Projects.projectno,
                    ActivityCategories.categoryname,
                    ActivitySubcategories.subcategoryname,
                    CONCAT(Employees.firstname, ' ', Employees.lastname) AS activity_manager,
                    Activities.plannedstart,
                    Activities.plannedfinish,
                    Activities.actualstart,
                    Activities.actualfinish,
                    COUNT(ActivityTasks.taskid) AS task_count
                FROM Activities
                INNER JOIN ActivityTasks ON Activities.activityid = ActivityTasks.activityid
                INNER JOIN ActivityTaskAssignments ON ActivityTasks.taskid = ActivityTaskAssignments.taskid
                INNER JOIN ActivitySubcategories ON Activities.subcategoryid = ActivitySubcategories.subcategoryid
                INNER JOIN ActivityCategories ON ActivitySubcategories.categoryid = ActivityCategories.categoryid
                INNER JOIN Projects ON ActivityCategories.projectid = Projects.projectid
                INNER JOIN Employees ON Activities.activitymanagerid = Employees.employeeid
                WHERE ActivityTaskAssignments.userid = ?
                GROUP BY Activities.activityid
                ORDER BY Projects.projectname, Activities.activityname";
        
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $employeeid);
        $stmt->execute();
        $result = $stmt->get_result();
        
        $data = [];
        while ($row = $result->fetch_assoc()) {
            $data[] = $row;
        }
        
        $stmt->close();
        return $data;
    }
    
    public function getUserTasksForTimeBooking($employeeid) {
        $sql = "SELECT
                    ActivityTasks.taskid,
                    ActivityTaskAssignments.assignmentid,
                    ActivityTasks.taskname AS Task_Name,
                    Activities.activityname AS Activity_Name,
                    Projects.projectname AS Project_Name,
                    COALESCE(SUM(Hours.hours), 0) AS Booked_Hours,
                    CASE 
                        WHEN ActivityTaskAssignments.closedate IS NULL THEN 'Open'
                        ELSE 'Closed'
                    END AS Status
                FROM ActivityTasks
                INNER JOIN ActivityTaskAssignments ON ActivityTasks.taskid = ActivityTaskAssignments.taskid
                INNER JOIN Activities ON ActivityTasks.activityid = Activities.activityid
                INNER JOIN ActivitySubcategories ON Activities.subcategoryid = ActivitySubcategories.subcategoryid
                INNER JOIN ActivityCategories ON ActivitySubcategories.categoryid = ActivityCategories.categoryid
                INNER JOIN Projects ON ActivityCategories.projectid = Projects.projectid
                LEFT JOIN Hours ON ActivityTaskAssignments.assignmentid = Hours.assignmentid
                WHERE ActivityTaskAssignments.userid = ? AND ActivityTaskAssignments.closedate IS NULL
                GROUP BY ActivityTasks.taskid, ActivityTaskAssignments.assignmentid
                ORDER BY Projects.projectname, Activities.activityname, ActivityTasks.taskname";
        
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $employeeid);
        $stmt->execute();
        $result = $stmt->get_result();
        
        $tasks = [];
        while ($row = $result->fetch_assoc()) {
            $row['Task_Hierarchy'] = $this->getTaskHierarchy($row['taskid']);
            $tasks[] = $row;
        }
        
        $stmt->close();
        return $tasks;
    }
    
    public function bookTime($assignmentid, $hours, $bookeddate) {
        $sql = "INSERT INTO Hours (assignmentid, hours, bookeddate) VALUES (?, ?, ?)";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('iis', $assignmentid, $hours, $bookeddate);
        $result = $stmt->execute();
        $stmt->close();
        return $result;
    }
    
    public function addActivityNote($activityid, $userid, $note, $notedate) {
        $sql = "INSERT INTO ActivityNotes (activityid, userid, note, notedate) VALUES (?, ?, ?, ?)";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('iiss', $activityid, $userid, $note, $notedate);
        $result = $stmt->execute();
        $stmt->close();
        return $result;
    }
    
    public function deleteActivityNote($noteid, $userid) {
        $sql = "DELETE FROM ActivityNotes WHERE noteid = ? AND userid = ?";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('ii', $noteid, $userid);
        $result = $stmt->execute();
        $stmt->close();
        return $result;
    }
    
    public function updateActivityNote($noteid, $userid, $note, $notedate) {
        $sql = "UPDATE ActivityNotes SET note = ?, notedate = ? WHERE noteid = ? AND userid = ?";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('ssii', $note, $notedate, $noteid, $userid);
        $result = $stmt->execute();
        $stmt->close();
        return $result;
    }

    public function getTaskHeirarchy($activityid){
        $sql = "SELECT
                    taskname,
                    ActivityTasks.taskid,
                    parenttaskid,
                    CASE
                    WHEN ISNULL(ActivityTaskAssignments.assigneddate) THEN 2
                    WHEN ISNULL(ActivityTaskAssignments.closedate) THEN 1
                    ELSE 0
                    END AS base_status
                FROM ActivityTasks
                LEFT JOIN ActivityTaskAssignments ON ActivityTasks.taskid = ActivityTaskAssignments.taskid AND ActivityTaskAssignments.closedate IS NULL
                WHERE activityid = ?;";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i',$activityid);
        $stmt->execute();
        $result = $stmt->get_result();
        $tasks = [];
        while ($row = $result->fetch_assoc()){
            $tasks[$row['taskid']] = $row;
        }
        $stmt->close();

        // Calculate hierarchical status and hours for each task
        foreach ($tasks as $taskid => $task) {
            $tasks[$taskid]['status'] = $this->calculateHierarchicalStatus($taskid, $tasks);
            $tasks[$taskid]['hours'] = $this->calculateTaskHours($taskid, $tasks);
        }

        return $tasks;
    }

    private function calculateHierarchicalStatus($taskid, $tasks) {
        $task = $tasks[$taskid];

        // Get all children of this task
        $children = [];
        foreach ($tasks as $childTask) {
            if ($childTask['parenttaskid'] == $taskid) {
                $children[] = $childTask;
            }
        }

        // If task has no children, use its base status
        if (empty($children)) {
            return $task['base_status'];
        }

        // If task has children, determine status based on children's status
        $childStatuses = [];
        foreach ($children as $child) {
            $childStatuses[] = $this->calculateHierarchicalStatus($child['taskid'], $tasks);
        }

        // If task is assigned to any user (base_status = 1), show as in-progress
        if ($task['base_status'] == 1) {
            return 1; // In-progress
        }

        // Determine status based on children
        $completeCount = 0;
        $openCount = 0;
        $inProgressCount = 0;

        foreach ($childStatuses as $status) {
            switch ($status) {
                case 0: $completeCount++; break;    // Complete
                case 1: $inProgressCount++; break;  // In-progress
                case 2: $openCount++; break;        // Not started
            }
        }

        // If all children are complete, mark as complete
        if ($completeCount == count($childStatuses)) {
            return 0; // Complete
        }

        // If all children are open (not started), mark as open
        if ($openCount == count($childStatuses)) {
            return 2; // Not started
        }

        // Otherwise, mark as in-progress
        return 1; // In-progress
    }

    private function calculateTaskHours($taskid, $tasks) {
        // Get direct hours for this task
        $directHours = $this->getDirectTaskHours($taskid);

        // Get all children of this task
        $children = [];
        foreach ($tasks as $childTask) {
            if ($childTask['parenttaskid'] == $taskid) {
                $children[] = $childTask;
            }
        }

        // If task has no children, return just direct hours
        if (empty($children)) {
            return $directHours;
        }

        // Calculate total hours including children
        $totalHours = $directHours;
        foreach ($children as $child) {
            $totalHours += $this->calculateTaskHours($child['taskid'], $tasks);
        }

        return $totalHours;
    }

    private function getDirectTaskHours($taskid) {
        $sql = "SELECT SUM(Hours.hours) AS total_hours
                FROM Hours
                INNER JOIN ActivityTaskAssignments ON Hours.assignmentid = ActivityTaskAssignments.assignmentid
                WHERE ActivityTaskAssignments.taskid = ?";

        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $taskid);
        $stmt->execute();
        $result = $stmt->get_result();
        $data = $result->fetch_assoc();
        $stmt->close();

        return $data['total_hours'] ?? 0;
    }

    public function getActivityName($activityid){
        $sql = 'SELECT activityname FROM Activities WHERE activityid=?;';
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i',$activityid);
        $stmt->execute();
        $result = $stmt->get_result();
        $activityname = $result->fetch_row()[0];
        $stmt->close();
        return $activityname;
    }

    public function isActivityManager($activityid, $userid) {
        $sql = "SELECT activitymanagerid FROM Activities WHERE activityid = ?";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $activityid);
        $stmt->execute();
        $result = $stmt->get_result();
        $data = $result->fetch_assoc();
        $stmt->close();

        return $data && $data['activitymanagerid'] == $userid;
    }

    public function getTasksForParentSelection($activityid) {
        $sql = "SELECT taskid, taskname, parenttaskid FROM ActivityTasks WHERE activityid = ? ORDER BY taskname";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $activityid);
        $stmt->execute();
        $result = $stmt->get_result();

        $tasks = [];
        while ($row = $result->fetch_assoc()) {
            $tasks[] = $row;
        }

        $stmt->close();
        return $tasks;
    }

    public function addActivityTask($activityid, $taskname, $parenttaskid = null) {
        $sql = "INSERT INTO ActivityTasks (activityid, taskname, parenttaskid) VALUES (?, ?, ?)";
        $stmt = $this->db->prepare($sql);

        // If parenttaskid is 0 or empty, set it to null for root level tasks
        $parenttaskid = ($parenttaskid == 0 || empty($parenttaskid)) ? null : $parenttaskid;

        $stmt->bind_param('isi', $activityid, $taskname, $parenttaskid);
        $result = $stmt->execute();
        $stmt->close();
        return $result;
    }

    public function deleteActivityTask($taskid) {
        // First check if task has children
        $sqlCheck = "SELECT COUNT(*) as child_count FROM ActivityTasks WHERE parenttaskid = ?";
        $stmtCheck = $this->db->prepare($sqlCheck);
        $stmtCheck->bind_param('i', $taskid);
        $stmtCheck->execute();
        $result = $stmtCheck->get_result();
        $data = $result->fetch_assoc();
        $stmtCheck->close();

        if ($data['child_count'] > 0) {
            return false; // Cannot delete task with children
        }

        // Delete task assignments first (foreign key constraint)
        $sqlDeleteAssignments = "DELETE FROM ActivityTaskAssignments WHERE taskid = ?";
        $stmtDeleteAssignments = $this->db->prepare($sqlDeleteAssignments);
        $stmtDeleteAssignments->bind_param('i', $taskid);
        $stmtDeleteAssignments->execute();
        $stmtDeleteAssignments->close();

        // Then delete the task
        $sql = "DELETE FROM ActivityTasks WHERE taskid = ?";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $taskid);
        $result = $stmt->execute();
        $stmt->close();
        return $result;
    }

    public function getUnassignedTasks($activityid) {
        $sql = "SELECT
                    AT.taskid,
                    AT.taskname
                FROM ActivityTasks AT
                LEFT JOIN ActivityTaskAssignments ATA ON AT.taskid = ATA.taskid AND ATA.closedate IS NULL
                WHERE AT.activityid = ? AND ATA.taskid IS NULL
                ORDER BY AT.taskname";

        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $activityid);
        $stmt->execute();
        $result = $stmt->get_result();

        $tasks = [];
        while ($row = $result->fetch_assoc()) {
            $tasks[] = $row;
        }

        $stmt->close();
        return $tasks;
    }

    public function getAllEmployees() {
        $sql = "SELECT employeeid, CONCAT(firstname, ' ', lastname) AS fullname
                FROM Employees
                ORDER BY firstname, lastname";

        $stmt = $this->db->prepare($sql);
        $stmt->execute();
        $result = $stmt->get_result();

        $employees = [];
        while ($row = $result->fetch_assoc()) {
            $employees[] = $row;
        }

        $stmt->close();
        return $employees;
    }

    public function assignTask($taskid, $userid, $assigneddate = null) {
        // Check if task is already assigned and open
        $sqlCheck = "SELECT assignmentid FROM ActivityTaskAssignments WHERE taskid = ? AND closedate IS NULL";
        $stmtCheck = $this->db->prepare($sqlCheck);
        $stmtCheck->bind_param('i', $taskid);
        $stmtCheck->execute();
        $result = $stmtCheck->get_result();

        if ($result->num_rows > 0) {
            $stmtCheck->close();
            return false; // Task is already assigned
        }
        $stmtCheck->close();

        // Set default date if not provided
        if ($assigneddate === null) {
            $assigneddate = date('Y-m-d');
        }

        // Create new assignment
        $sql = "INSERT INTO ActivityTaskAssignments (taskid, userid, assigneddate) VALUES (?, ?, ?)";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('iis', $taskid, $userid, $assigneddate);
        $result = $stmt->execute();
        $stmt->close();

        return $result;
    }

    public function getTaskAssignments($activityid) {
        $sql = "SELECT
                    AT.taskid,
                    AT.taskname,
                    CONCAT(E.firstname, ' ', E.lastname) AS assignee,
                    ATA.assigneddate,
                    ATA.closedate,
                    CASE
                        WHEN ATA.closedate IS NULL THEN 'Open'
                        ELSE 'Closed'
                    END AS status
                FROM ActivityTasks AT
                INNER JOIN ActivityTaskAssignments ATA ON AT.taskid = ATA.taskid
                INNER JOIN Employees E ON ATA.userid = E.employeeid
                WHERE AT.activityid = ?
                ORDER BY AT.taskname, ATA.assigneddate DESC";

        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $activityid);
        $stmt->execute();
        $result = $stmt->get_result();

        $assignments = [];
        while ($row = $result->fetch_assoc()) {
            $assignments[] = $row;
        }

        $stmt->close();
        return $assignments;
    }

    public function isProjectManager($activityid, $userid) {
        $sql = "SELECT Projects.projectmanagerid
                FROM Activities
                INNER JOIN ActivitySubcategories ON Activities.subcategoryid = ActivitySubcategories.subcategoryid
                INNER JOIN ActivityCategories ON ActivitySubcategories.categoryid = ActivityCategories.categoryid
                INNER JOIN Projects ON ActivityCategories.projectid = Projects.projectid
                WHERE Activities.activityid = ?";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $activityid);
        $stmt->execute();
        $result = $stmt->get_result();
        $data = $result->fetch_assoc();
        $stmt->close();

        return $data && $data['projectmanagerid'] == $userid;
    }

    public function updateActivity($activityid, $activityname, $activitydescription, $plannedstart, $plannedfinish, $forecaststart, $forecastfinish, $actualstart, $actualfinish, $plannedhours, $forecasthours) {
        $sql = "UPDATE Activities SET
                    activityname = ?,
                    activitydescription = ?,
                    plannedstart = ?,
                    plannedfinish = ?,
                    forecaststart = ?,
                    forecastfinish = ?,
                    actualstart = ?,
                    actualfinish = ?,
                    plannedhours = ?,
                    forecasthours = ?
                WHERE activityid = ?";

        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('ssssssssddi',
            $activityname,
            $activitydescription,
            $plannedstart,
            $plannedfinish,
            $forecaststart,
            $forecastfinish,
            $actualstart,
            $actualfinish,
            $plannedhours,
            $forecasthours,
            $activityid
        );
        $result = $stmt->execute();
        $stmt->close();
        return $result;
    }

    public function getActivityManagerId($activityid) {
        $sql = "SELECT activitymanagerid FROM Activities WHERE activityid = ?";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $activityid);
        $stmt->execute();
        $result = $stmt->get_result();
        $data = $result->fetch_assoc();
        $stmt->close();

        return $data ? $data['activitymanagerid'] : null;
    }

    public function reassignTask($assignmentid, $newuserid, $reassigndate = null) {
        // Set default date if not provided
        if ($reassigndate === null) {
            $reassigndate = date('Y-m-d');
        }

        // First, get the task information from the current assignment
        $sqlGetTask = "SELECT taskid FROM ActivityTaskAssignments WHERE assignmentid = ? AND closedate IS NULL";
        $stmtGetTask = $this->db->prepare($sqlGetTask);
        $stmtGetTask->bind_param('i', $assignmentid);
        $stmtGetTask->execute();
        $result = $stmtGetTask->get_result();
        $taskData = $result->fetch_assoc();
        $stmtGetTask->close();

        if (!$taskData) {
            return false; // Assignment not found or already closed
        }

        $taskid = $taskData['taskid'];

        // Check if the new user already has an open assignment for this task
        $sqlCheckExisting = "SELECT assignmentid FROM ActivityTaskAssignments WHERE taskid = ? AND userid = ? AND closedate IS NULL";
        $stmtCheckExisting = $this->db->prepare($sqlCheckExisting);
        $stmtCheckExisting->bind_param('ii', $taskid, $newuserid);
        $stmtCheckExisting->execute();
        $existingResult = $stmtCheckExisting->get_result();

        if ($existingResult->num_rows > 0) {
            $stmtCheckExisting->close();
            return false; // User already has an open assignment for this task
        }
        $stmtCheckExisting->close();

        // Close the current assignment
        $sqlCloseOld = "UPDATE ActivityTaskAssignments SET closedate = ? WHERE assignmentid = ?";
        $stmtCloseOld = $this->db->prepare($sqlCloseOld);
        $stmtCloseOld->bind_param('si', $reassigndate, $assignmentid);
        $result1 = $stmtCloseOld->execute();
        $stmtCloseOld->close();

        if (!$result1) {
            return false;
        }

        // Create new assignment
        $sqlNewAssignment = "INSERT INTO ActivityTaskAssignments (taskid, userid, assigneddate) VALUES (?, ?, ?)";
        $stmtNewAssignment = $this->db->prepare($sqlNewAssignment);
        $stmtNewAssignment->bind_param('iis', $taskid, $newuserid, $reassigndate);
        $result2 = $stmtNewAssignment->execute();
        $stmtNewAssignment->close();

        return $result2;
    }

    public function closeTask($assignmentid, $userid, $activityid, $closedate = null) {
        // Set default date if not provided
        if ($closedate === null) {
            $closedate = date('Y-m-d');
        }

        // Check if assignment exists and is open
        $sqlCheck = "SELECT taskid FROM ActivityTaskAssignments WHERE assignmentid = ? AND closedate IS NULL";
        $stmtCheck = $this->db->prepare($sqlCheck);
        $stmtCheck->bind_param('i', $assignmentid);
        $stmtCheck->execute();
        $result = $stmtCheck->get_result();
        $taskData = $result->fetch_assoc();
        $stmtCheck->close();

        if (!$taskData) {
            return ['success' => false, 'message' => 'Assignment not found or already closed'];
        }

        $taskid = $taskData['taskid'];

        // Check if user is activity manager
        if ($this->isActivityManager($activityid, $userid)) {
            // User is activity manager - just close the task
            $sqlClose = "UPDATE ActivityTaskAssignments SET closedate = ? WHERE assignmentid = ?";
            $stmtClose = $this->db->prepare($sqlClose);
            $stmtClose->bind_param('si', $closedate, $assignmentid);
            $result = $stmtClose->execute();
            $stmtClose->close();

            if ($result) {
                return ['success' => true, 'action' => 'closed', 'message' => 'Task closed successfully'];
            } else {
                return ['success' => false, 'message' => 'Error closing task'];
            }
        } else {
            // User is not activity manager - reassign to activity manager
            $activityManagerId = $this->getActivityManagerId($activityid);

            if (!$activityManagerId) {
                return ['success' => false, 'message' => 'Activity manager not found'];
            }

            // Check if activity manager already has this task assigned
            $sqlCheckManager = "SELECT assignmentid FROM ActivityTaskAssignments WHERE taskid = ? AND userid = ? AND closedate IS NULL";
            $stmtCheckManager = $this->db->prepare($sqlCheckManager);
            $stmtCheckManager->bind_param('ii', $taskid, $activityManagerId);
            $stmtCheckManager->execute();
            $managerResult = $stmtCheckManager->get_result();

            if ($managerResult->num_rows > 0) {
                $stmtCheckManager->close();
                return ['success' => false, 'message' => 'Activity manager already has this task assigned'];
            }
            $stmtCheckManager->close();

            // Close current assignment and create new one for activity manager
            $sqlCloseOld = "UPDATE ActivityTaskAssignments SET closedate = ? WHERE assignmentid = ?";
            $stmtCloseOld = $this->db->prepare($sqlCloseOld);
            $stmtCloseOld->bind_param('si', $closedate, $assignmentid);
            $result1 = $stmtCloseOld->execute();
            $stmtCloseOld->close();

            if (!$result1) {
                return ['success' => false, 'message' => 'Error closing current assignment'];
            }

            // Create new assignment for activity manager
            $sqlNewAssignment = "INSERT INTO ActivityTaskAssignments (taskid, userid, assigneddate) VALUES (?, ?, ?)";
            $stmtNewAssignment = $this->db->prepare($sqlNewAssignment);
            $stmtNewAssignment->bind_param('iis', $taskid, $activityManagerId, $closedate);
            $result2 = $stmtNewAssignment->execute();
            $stmtNewAssignment->close();

            if ($result2) {
                return ['success' => true, 'action' => 'reassigned', 'message' => 'Task completed and reassigned to activity manager'];
            } else {
                return ['success' => false, 'message' => 'Error reassigning task to activity manager'];
            }
        }
    }
}
