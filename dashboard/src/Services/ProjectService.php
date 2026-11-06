<?php

require_once __DIR__ . '/../Database.php';

class ProjectService {
    private $db;

    // ProjectTeamMembers.role values
    const ROLE_NAMES = [0 => 'Member', 1 => 'Lead', 2 => 'Project Manager'];

    public function __construct() {
        $this->db = Database::getInstance();
    }

    // Team members of a project, managers first
    public function getProjectTeamMembers($projectid) {
        $sql = "SELECT Employees.employeeid,
                       CONCAT(Employees.firstname, ' ', Employees.lastname) AS fullname,
                       Employees.email,
                       Departments.departmentname,
                       ProjectTeamMembers.role
                FROM ProjectTeamMembers
                INNER JOIN Employees ON ProjectTeamMembers.employeeid = Employees.employeeid
                LEFT JOIN Departments ON Employees.departmentid = Departments.departmentid
                WHERE ProjectTeamMembers.projectid = ?
                ORDER BY ProjectTeamMembers.role DESC, Employees.firstname, Employees.lastname";

        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $projectid);
        $stmt->execute();
        $result = $stmt->get_result();

        $data = [];
        while ($row = $result->fetch_assoc()) {
            $data[] = $row;
        }

        $stmt->close();
        return $data;
    }

    // Employees who are not yet members of the project, for the add team member form
    public function getNonTeamMembers($projectid) {
        $sql = "SELECT Employees.employeeid,
                       CONCAT(Employees.firstname, ' ', Employees.lastname) AS fullname,
                       Departments.departmentname
                FROM Employees
                LEFT JOIN Departments ON Employees.departmentid = Departments.departmentid
                WHERE Employees.employeeid NOT IN (
                    SELECT employeeid FROM ProjectTeamMembers WHERE projectid = ?
                )
                ORDER BY Employees.firstname, Employees.lastname";

        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $projectid);
        $stmt->execute();
        $result = $stmt->get_result();

        $data = [];
        while ($row = $result->fetch_assoc()) {
            $data[] = $row;
        }

        $stmt->close();
        return $data;
    }

    // Adds an employee to the project team. Returns false if the employee is already a member,
    // does not exist, or the insert fails.
    public function addTeamMember($projectid, $employeeid, $role) {
        if (!array_key_exists((int)$role, self::ROLE_NAMES)) {
            return false;
        }

        $sql = "INSERT INTO ProjectTeamMembers (projectid, employeeid, role)
                SELECT ?, Employees.employeeid, ?
                FROM Employees
                WHERE Employees.employeeid = ?
                  AND NOT EXISTS (
                      SELECT 1 FROM ProjectTeamMembers WHERE projectid = ? AND employeeid = ?
                  )";

        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('iiiii', $projectid, $role, $employeeid, $projectid, $employeeid);
        $success = $stmt->execute() && $stmt->affected_rows === 1;
        $stmt->close();

        return $success;
    }

    // True if another project already uses this project number (Projects.projectno is UNIQUE)
    public function projectNumberExists($projectno, $excludeprojectid) {
        $stmt = $this->db->prepare("SELECT 1 FROM Projects WHERE projectno = ? AND projectid <> ?");
        $stmt->bind_param('si', $projectno, $excludeprojectid);
        $stmt->execute();
        $stmt->store_result();
        $exists = $stmt->num_rows > 0;
        $stmt->close();

        return $exists;
    }

    public function updateProject($projectid, $projectno, $projectname) {
        try {
            $stmt = $this->db->prepare("UPDATE Projects SET projectno = ?, projectname = ? WHERE projectid = ?");
            $stmt->bind_param('ssi', $projectno, $projectname, $projectid);
            $success = $stmt->execute();
            $stmt->close();
            return $success;
        } catch (mysqli_sql_exception $e) {
            return false;
        }
    }

    public function isProjectManager($projectid, $userid) {
        $sql = "SELECT 1 FROM ProjectTeamMembers WHERE projectid = ? AND employeeid = ? AND role = 2";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('ii', $projectid, $userid);
        $stmt->execute();
        $stmt->store_result();
        $isManager = $stmt->num_rows > 0;
        $stmt->close();

        return $isManager;
    }

    // True if the user is a Lead (role 1) or Manager (role 2) on the project team
    public function isProjectLeadOrManager($projectid, $userid) {
        $sql = "SELECT 1 FROM ProjectTeamMembers WHERE projectid = ? AND employeeid = ? AND role IN (1, 2)";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('ii', $projectid, $userid);
        $stmt->execute();
        $stmt->store_result();
        $isLeadOrManager = $stmt->num_rows > 0;
        $stmt->close();

        return $isLeadOrManager;
    }

    public function getSubcategories($projectid) {
        $sql = "SELECT ActivitySubcategories.subcategoryid,
                       ActivitySubcategories.subcategoryname,
                       ActivityCategories.categoryname
                FROM ActivitySubcategories
                INNER JOIN ActivityCategories ON ActivitySubcategories.categoryid = ActivityCategories.categoryid
                WHERE ActivityCategories.projectid = ?
                ORDER BY ActivityCategories.categoryname, ActivitySubcategories.subcategoryname";

        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $projectid);
        $stmt->execute();
        $result = $stmt->get_result();

        $data = [];
        while ($row = $result->fetch_assoc()) {
            $data[] = $row;
        }

        $stmt->close();
        return $data;
    }

    public function getCategories($projectid) {
        $sql = "SELECT categoryid, categoryname FROM ActivityCategories WHERE projectid = ? ORDER BY categoryname";
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $projectid);
        $stmt->execute();
        $result = $stmt->get_result();

        $data = [];
        while ($row = $result->fetch_assoc()) {
            $data[] = $row;
        }

        $stmt->close();
        return $data;
    }

    // Reuses an existing category/subcategory with the same name, otherwise creates it.
    // Returns the subcategoryid, or false on failure.
    public function findOrCreateSubcategory($projectid, $categoryname, $subcategoryname) {
        $stmt = $this->db->prepare("SELECT categoryid FROM ActivityCategories WHERE projectid = ? AND categoryname = ?");
        $stmt->bind_param('is', $projectid, $categoryname);
        $stmt->execute();
        $row = $stmt->get_result()->fetch_assoc();
        $stmt->close();

        if ($row) {
            $categoryid = (int)$row['categoryid'];
        } else {
            $stmt = $this->db->prepare("INSERT INTO ActivityCategories (projectid, categoryname) VALUES (?, ?)");
            $stmt->bind_param('is', $projectid, $categoryname);
            $categoryid = $stmt->execute() ? $stmt->insert_id : false;
            $stmt->close();
            if (!$categoryid) {
                return false;
            }
        }

        $stmt = $this->db->prepare("SELECT subcategoryid FROM ActivitySubcategories WHERE categoryid = ? AND subcategoryname = ?");
        $stmt->bind_param('is', $categoryid, $subcategoryname);
        $stmt->execute();
        $row = $stmt->get_result()->fetch_assoc();
        $stmt->close();

        if ($row) {
            return (int)$row['subcategoryid'];
        }

        $stmt = $this->db->prepare("INSERT INTO ActivitySubcategories (categoryid, subcategoryname) VALUES (?, ?)");
        $stmt->bind_param('is', $categoryid, $subcategoryname);
        $subcategoryid = $stmt->execute() ? $stmt->insert_id : false;
        $stmt->close();
        return $subcategoryid;
    }

    public static function roleName($role) {
        return self::ROLE_NAMES[(int)$role] ?? 'Member';
    }
}
