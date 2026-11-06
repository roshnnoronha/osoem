<?php

require_once __DIR__ . '/../Database.php';

class ActivityService {
    private $db;
    
    public function __construct() {
        $this->db = Database::getInstance();
    }

    // Actual start/finish are derived from task assignments rather than entered manually:
    //   start  = earliest assignment date across the activity's tasks
    //   finish = latest close date, once every assignment is closed and every leaf task has been assigned
    private static function actualStartSql() {
        return "(SELECT MIN(a.assigneddate)
                 FROM ActivityTaskAssignments a
                 INNER JOIN ActivityTasks t ON a.taskid = t.taskid
                 WHERE t.activityid = Activities.activityid)";
    }

    private static function actualFinishSql() {
        return "(CASE
                    WHEN EXISTS (
                        SELECT 1 FROM ActivityTaskAssignments a
                        INNER JOIN ActivityTasks t ON a.taskid = t.taskid
                        WHERE t.activityid = Activities.activityid AND a.closedate IS NULL
                    ) THEN NULL
                    WHEN EXISTS (
                        SELECT 1 FROM ActivityTasks t
                        WHERE t.activityid = Activities.activityid
                          AND NOT EXISTS (SELECT 1 FROM ActivityTasks c WHERE c.parenttaskid = t.taskid)
                          AND NOT EXISTS (SELECT 1 FROM ActivityTaskAssignments a WHERE a.taskid = t.taskid)
                    ) THEN NULL
                    ELSE (
                        SELECT MAX(a.closedate) FROM ActivityTaskAssignments a
                        INNER JOIN ActivityTasks t ON a.taskid = t.taskid
                        WHERE t.activityid = Activities.activityid
                    )
                 END)";
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
                    Activities.activityname AS activity_name,
                    Activities.activitydescription AS description,
                    Projects.projectno AS project_number,
                    Projects.projectname AS project_name,
                    ActivityCategories.categoryname AS category,
                    ActivitySubcategories.subcategoryname AS subcategory,
                    CONCAT(Employees.firstname,' ' ,Employees.lastname) AS activity_manager,
                    Activities.plannedstart AS planned_start,
                    Activities.plannedfinish AS planned_finish,
                    Activities.forecaststart AS forecast_start,
                    Activities.forecastfinish AS forecast_finish,
                    " . self::actualStartSql() . " AS actual_start,
                    " . self::actualFinishSql() . " AS actual_finish,
                    Activities.plannedhours AS planned_hours,
                    Activities.forecasthours AS forecast_hours,
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
                    SUM(Hours.hours) AS consumed_hours
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
        
        return $data['consumed_hours'] ?? 0;
    }
    
    public function getActivityNotes($activityid) {
        $sql = "SELECT
                   ActivityNotes.noteid AS note_id,
                   ActivityNotes.userid AS user_id,
                   note,
                   concat(Employees.firstname,' ' ,Employees.lastname) AS note_by,
                   notedate AS note_date
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
                    ActivityTasks.taskname AS task_name,
                    concat(Employees.firstname,' ' ,Employees.lastname) AS assigned_to,
                    ActivityTaskAssignments.assigneddate AS assigned_date,
                    ActivityTaskAssignments.closedate AS closed_date,
                    COALESCE(SUM(Hours.hours), 0) AS booked_hours,
                    CASE
                        WHEN ActivityTaskAssignments.closedate IS NULL THEN 'Open'
                        ELSE 'Closed'
                    END AS status
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
            $row['task_hierarchy'] = $this->getTaskHierarchy($row['taskid']);
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
                    Projects.projectid,
                    Projects.projectname,
                    Projects.projectno,
                    ActivityCategories.categoryname,
                    ActivitySubcategories.subcategoryname,
                    CONCAT(Employees.firstname, ' ', Employees.lastname) AS activity_manager,
                    Activities.plannedstart,
                    Activities.plannedfinish,
                    " . self::actualStartSql() . " AS actualstart,
                    " . self::actualFinishSql() . " AS actualfinish,
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
    
    // Activities in a project. When $employeeid is given, only those where the user
    // is the activity manager or has been assigned a task.
    public function getProjectActivities($projectid, $employeeid = null) {
        $sql = "SELECT
                    Activities.activityid,
                    Activities.activityname,
                    Activities.activitydescription,
                    Projects.projectid,
                    ActivityCategories.categoryname,
                    ActivitySubcategories.subcategoryname,
                    CONCAT(Employees.firstname, ' ', Employees.lastname) AS activity_manager,
                    Activities.plannedstart,
                    Activities.plannedfinish,
                    " . self::actualStartSql() . " AS actualstart,
                    " . self::actualFinishSql() . " AS actualfinish
                FROM Activities
                INNER JOIN ActivitySubcategories ON Activities.subcategoryid = ActivitySubcategories.subcategoryid
                INNER JOIN ActivityCategories ON ActivitySubcategories.categoryid = ActivityCategories.categoryid
                INNER JOIN Projects ON ActivityCategories.projectid = Projects.projectid
                INNER JOIN Employees ON Activities.activitymanagerid = Employees.employeeid
                WHERE Projects.projectid = ?";
        if ($employeeid !== null) {
            $sql .= " AND (Activities.activitymanagerid = ?
                           OR EXISTS (
                               SELECT 1 FROM ActivityTaskAssignments a
                               INNER JOIN ActivityTasks t ON a.taskid = t.taskid
                               WHERE t.activityid = Activities.activityid AND a.userid = ?
                           ))";
        }
        $sql .= " ORDER BY Activities.activityname";

        $stmt = $this->db->prepare($sql);
        if ($employeeid !== null) {
            $stmt->bind_param('iii', $projectid, $employeeid, $employeeid);
        } else {
            $stmt->bind_param('i', $projectid);
        }
        $stmt->execute();
        $result = $stmt->get_result();

        $data = [];
        while ($row = $result->fetch_assoc()) {
            $data[] = $row;
        }

        $stmt->close();
        return $data;
    }

    // Projects the user is a team member of, or has task assignments in
    public function getUserProjects($employeeid) {
        $sql = "SELECT Projects.projectid, Projects.projectno, Projects.projectname
                FROM Projects
                WHERE Projects.projectid IN (
                    SELECT ProjectTeamMembers.projectid
                    FROM ProjectTeamMembers
                    WHERE ProjectTeamMembers.employeeid = ?
                    UNION
                    SELECT ActivityCategories.projectid
                    FROM ActivityTaskAssignments
                    INNER JOIN ActivityTasks ON ActivityTaskAssignments.taskid = ActivityTasks.taskid
                    INNER JOIN Activities ON ActivityTasks.activityid = Activities.activityid
                    INNER JOIN ActivitySubcategories ON Activities.subcategoryid = ActivitySubcategories.subcategoryid
                    INNER JOIN ActivityCategories ON ActivitySubcategories.categoryid = ActivityCategories.categoryid
                    WHERE ActivityTaskAssignments.userid = ?
                )
                ORDER BY Projects.projectname";

        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('ii', $employeeid, $employeeid);
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
                    ActivityTasks.taskname AS task_name,
                    Activities.activityname AS activity_name,
                    Projects.projectname AS project_name,
                    COALESCE(SUM(Hours.hours), 0) AS booked_hours,
                    CASE
                        WHEN ActivityTaskAssignments.closedate IS NULL THEN 'Open'
                        ELSE 'Closed'
                    END AS status
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
            $row['task_hierarchy'] = $this->getTaskHierarchy($row['taskid']);
            $tasks[] = $row;
        }

        $stmt->close();
        return $tasks;
    }

    // Books hours against an assignment only if it is an open assignment belonging to $employeeid.
    // Returns true if a booking was recorded.
    public function bookTime($assignmentid, $employeeid, $hours, $bookeddate) {
        $sql = "INSERT INTO Hours (assignmentid, hours, bookeddate)
                SELECT assignmentid, ?, ?
                FROM ActivityTaskAssignments
                WHERE assignmentid = ? AND userid = ? AND closedate IS NULL";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('dsii', $hours, $bookeddate, $assignmentid, $employeeid);
        $result = $stmt->execute() && $stmt->affected_rows === 1;
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
    
    public function updateActivityNote($noteid, $userid, $note) {
        $sql = "UPDATE ActivityNotes SET note = ? WHERE noteid = ? AND userid = ?";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('sii', $note, $noteid, $userid);
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
                    WHEN NOT EXISTS (
                        SELECT 1 FROM ActivityTaskAssignments a WHERE a.taskid = ActivityTasks.taskid
                    ) THEN " . TaskStatus::NOT_STARTED . "
                    WHEN EXISTS (
                        SELECT 1 FROM ActivityTaskAssignments a WHERE a.taskid = ActivityTasks.taskid AND a.closedate IS NULL
                    ) THEN " . TaskStatus::IN_PROGRESS . "
                    ELSE " . TaskStatus::COMPLETE . "
                    END AS base_status
                FROM ActivityTasks
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

        // If task is assigned to any user (base_status = IN_PROGRESS), show as in-progress
        if ($task['base_status'] == TaskStatus::IN_PROGRESS) {
            return TaskStatus::IN_PROGRESS;
        }

        // Determine status based on children
        $completeCount = 0;
        $openCount = 0;
        $inProgressCount = 0;

        foreach ($childStatuses as $status) {
            switch ($status) {
                case TaskStatus::COMPLETE: $completeCount++; break;
                case TaskStatus::IN_PROGRESS: $inProgressCount++; break;
                case TaskStatus::NOT_STARTED: $openCount++; break;
            }
        }

        // If all children are complete, mark as complete
        if ($completeCount == count($childStatuses)) {
            return TaskStatus::COMPLETE;
        }

        // If all children are open (not started), mark as open
        if ($openCount == count($childStatuses)) {
            return TaskStatus::NOT_STARTED;
        }

        // Otherwise, mark as in-progress
        return TaskStatus::IN_PROGRESS;
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

    public function isProjectTeamMember($activityid, $userid) {
        $sql = "SELECT 1
                FROM Activities
                INNER JOIN ActivitySubcategories ON Activities.subcategoryid = ActivitySubcategories.subcategoryid
                INNER JOIN ActivityCategories ON ActivitySubcategories.categoryid = ActivityCategories.categoryid
                INNER JOIN ProjectTeamMembers ON ActivityCategories.projectid = ProjectTeamMembers.projectid
                WHERE Activities.activityid = ? AND ProjectTeamMembers.employeeid = ?";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('ii', $activityid, $userid);
        $stmt->execute();
        $stmt->store_result();
        $isMember = $stmt->num_rows > 0;
        $stmt->close();

        return $isMember;
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

    // Members of the project team that owns the activity, ordered by name
    public function getActivityTeamMembers($activityid) {
        $sql = "SELECT Employees.employeeid, CONCAT(Employees.firstname, ' ', Employees.lastname) AS fullname
                FROM Activities
                INNER JOIN ActivitySubcategories ON Activities.subcategoryid = ActivitySubcategories.subcategoryid
                INNER JOIN ActivityCategories ON ActivitySubcategories.categoryid = ActivityCategories.categoryid
                INNER JOIN ProjectTeamMembers ON ActivityCategories.projectid = ProjectTeamMembers.projectid
                INNER JOIN Employees ON ProjectTeamMembers.employeeid = Employees.employeeid
                WHERE Activities.activityid = ?
                ORDER BY Employees.firstname, Employees.lastname";

        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $activityid);
        $stmt->execute();
        $result = $stmt->get_result();

        $members = [];
        while ($row = $result->fetch_assoc()) {
            $members[] = $row;
        }

        $stmt->close();
        return $members;
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

    // Open assignments for every task in the activity, with the hierarchical task name and assignee
    public function getOpenTaskAssignments($activityid) {
        $sql = "SELECT
                    ATA.assignmentid,
                    ATA.userid,
                    AT.taskid,
                    CONCAT(E.firstname, ' ', E.lastname) AS assignee
                FROM ActivityTasks AT
                INNER JOIN ActivityTaskAssignments ATA ON AT.taskid = ATA.taskid
                INNER JOIN Employees E ON ATA.userid = E.employeeid
                WHERE AT.activityid = ? AND ATA.closedate IS NULL";

        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $activityid);
        $stmt->execute();
        $result = $stmt->get_result();

        $assignments = [];
        while ($row = $result->fetch_assoc()) {
            $row['task_hierarchy'] = $this->getTaskHierarchy($row['taskid']);
            $assignments[] = $row;
        }
        $stmt->close();

        usort($assignments, fn($a, $b) => strcmp($a['task_hierarchy'], $b['task_hierarchy']) ?: strcmp($a['assignee'], $b['assignee']));
        return $assignments;
    }

    public function getTaskDetails($taskid) {
        $sql = "SELECT
                    ActivityTasks.taskid,
                    ActivityTasks.activityid,
                    ActivityTasks.taskname,
                    ActivityTasks.taskdescription,
                    ActivityTasks.parenttaskid,
                    ActivityTasks.departmentid,
                    Departments.departmentname
                FROM ActivityTasks
                LEFT JOIN Departments ON ActivityTasks.departmentid = Departments.departmentid
                WHERE ActivityTasks.taskid = ?";

        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $taskid);
        $stmt->execute();
        $result = $stmt->get_result();
        $data = $result->fetch_assoc();
        $stmt->close();

        if ($data) {
            $data['task_hierarchy'] = $this->getTaskHierarchy($taskid);
        }

        return $data;
    }

    public function getTaskAssignmentHistory($taskid) {
        $sql = "SELECT
                    ATA.assignmentid,
                    ATA.userid,
                    CONCAT(E.firstname, ' ', E.lastname) AS assignee,
                    ATA.assigneddate,
                    ATA.closedate,
                    COALESCE(SUM(H.hours), 0) AS booked_hours,
                    CASE
                        WHEN ATA.closedate IS NULL THEN 'Open'
                        ELSE 'Closed'
                    END AS status
                FROM ActivityTaskAssignments ATA
                INNER JOIN Employees E ON ATA.userid = E.employeeid
                LEFT JOIN Hours H ON ATA.assignmentid = H.assignmentid
                WHERE ATA.taskid = ?
                GROUP BY ATA.assignmentid
                ORDER BY ATA.assigneddate DESC, ATA.assignmentid DESC";

        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $taskid);
        $stmt->execute();
        $result = $stmt->get_result();

        $assignments = [];
        while ($row = $result->fetch_assoc()) {
            $assignments[] = $row;
        }

        $stmt->close();
        return $assignments;
    }

    // True if the user is a Lead (role 1) or Manager (role 2) on the activity's project team
    public function isProjectLeadOrManager($activityid, $userid) {
        $sql = "SELECT 1
                FROM Activities
                INNER JOIN ActivitySubcategories ON Activities.subcategoryid = ActivitySubcategories.subcategoryid
                INNER JOIN ActivityCategories ON ActivitySubcategories.categoryid = ActivityCategories.categoryid
                INNER JOIN ProjectTeamMembers ON ActivityCategories.projectid = ProjectTeamMembers.projectid
                WHERE Activities.activityid = ? AND ProjectTeamMembers.employeeid = ? AND ProjectTeamMembers.role IN (1, 2)";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('ii', $activityid, $userid);
        $stmt->execute();
        $stmt->store_result();
        $isLeadOrManager = $stmt->num_rows > 0;
        $stmt->close();

        return $isLeadOrManager;
    }

    public function getDepartments() {
        $stmt = $this->db->prepare("SELECT departmentid, departmentname FROM Departments ORDER BY departmentname");
        $stmt->execute();
        $result = $stmt->get_result();
        $departments = [];
        while ($row = $result->fetch_assoc()) {
            $departments[] = $row;
        }
        $stmt->close();
        return $departments;
    }

    public function updateTaskDetails($taskid, $taskname, $taskdescription, $departmentid) {
        $sql = "UPDATE ActivityTasks SET taskname = ?, taskdescription = ?, departmentid = ? WHERE taskid = ?";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('ssii', $taskname, $taskdescription, $departmentid, $taskid);
        $result = $stmt->execute();
        $stmt->close();
        return $result;
    }

    public function isProjectManager($activityid, $userid) {
        // Project managers are ProjectTeamMembers with role 2 (0=Member, 1=Lead, 2=Manager)
        $sql = "SELECT 1
                FROM Activities
                INNER JOIN ActivitySubcategories ON Activities.subcategoryid = ActivitySubcategories.subcategoryid
                INNER JOIN ActivityCategories ON ActivitySubcategories.categoryid = ActivityCategories.categoryid
                INNER JOIN ProjectTeamMembers ON ActivityCategories.projectid = ProjectTeamMembers.projectid
                WHERE Activities.activityid = ? AND ProjectTeamMembers.employeeid = ? AND ProjectTeamMembers.role = 2";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('ii', $activityid, $userid);
        $stmt->execute();
        $stmt->store_result();
        $isManager = $stmt->num_rows > 0;
        $stmt->close();

        return $isManager;
    }

    // Returns the new activityid, or false on failure
    public function addActivity($subcategoryid, $activityname, $activitydescription, $activitymanagerid, $plannedstart, $plannedfinish, $plannedhours) {
        $sql = "INSERT INTO Activities (subcategoryid, activityname, activitydescription, activitymanagerid, plannedstart, plannedfinish, forecaststart, forecastfinish, plannedhours, forecasthours)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        $stmt = $this->db->prepare($sql);
        // Forecast starts out equal to the plan
        $stmt->bind_param('ississssdd', $subcategoryid, $activityname, $activitydescription, $activitymanagerid, $plannedstart, $plannedfinish, $plannedstart, $plannedfinish, $plannedhours, $plannedhours);
        $result = $stmt->execute();
        $activityid = $result ? $stmt->insert_id : false;
        $stmt->close();
        return $activityid;
    }

    public function updateActivity($activityid,$activityname, $activitydescription, $plannedstart, $plannedfinish, $forecaststart, $forecastfinish, $plannedhours, $forecasthours) {
        $sql = "UPDATE Activities SET
                    activityname = ?,
                    activitydescription = ?,
                    plannedstart = ?,
                    plannedfinish = ?,
                    forecaststart = ?,
                    forecastfinish = ?,
                    plannedhours = ?,
                    forecasthours = ?
                WHERE activityid = ?";

        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('ssssssddi',
            $activityname,
            $activitydescription,
            $plannedstart,
            $plannedfinish,
            $forecaststart,
            $forecastfinish,
            $plannedhours,
            $forecasthours,
            $activityid
        );
        $result = $stmt->execute();
        $stmt->close();
        return $result;
    }

    public function updateActivityManager($activityid, $activitymanagerid) {
        $sql = "UPDATE Activities SET activitymanagerid = ? WHERE activityid = ?";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('ii', $activitymanagerid, $activityid);
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

    // Closes every open assignment on a task, which closes the task itself.
    // Returns the number of assignments closed, or false on failure.
    public function closeAllTaskAssignments($taskid, $closedate = null) {
        if ($closedate === null) {
            $closedate = date('Y-m-d');
        }
        $sql = "UPDATE ActivityTaskAssignments SET closedate = ? WHERE taskid = ? AND closedate IS NULL";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('si', $closedate, $taskid);
        $result = $stmt->execute();
        $closed = $result ? $stmt->affected_rows : false;
        $stmt->close();
        return $closed;
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
