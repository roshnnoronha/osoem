<?php

require_once __DIR__ . '/../Database.php';

class AuthService {
    private $db;
    
    public function __construct() {
        $this->db = Database::getInstance();
    }
    
    public function authenticate($email, $password) {
        if (strlen($email) < 1 || strlen($password) < 1) {
            return ['success' => false, 'message' => 'User email and password are required to login.'];
        }

        $sql = 'SELECT password, passwordsalt, employeeid FROM Employees WHERE email=?;';
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('s', $email);
        $stmt->execute();
        $stmt->store_result();

        if ($stmt->num_rows == 0) {
            $stmt->close();
            return ['success' => false, 'message' => 'Invalid email id.'];
        }

        $stmt->bind_result($stored_password, $stored_salt, $eid);
        $stmt->fetch();

        if ($this->verifyPassword($password, $stored_password, $stored_salt)) {
            $stmt->close();
            return ['success' => true, 'employeeid' => $eid];
        } else {
            $stmt->close();
            return ['success' => false, 'message' => 'Incorrect password.'];
        }
    }

    // Matches admin/src/utils/auth.cpp::hashPassword() exactly (SHA-256(password + salt), lowercase hex)
    // so passwords set via the admin CLI tool can be verified here, and vice versa.
    private function hashPassword($password, $salt) {
        return hash('sha256', $password . $salt);
    }

    // Matches admin/src/utils/auth.cpp::authenticate()'s branching: an empty stored salt means
    // a legacy plaintext password (e.g. the temporary admin seed account).
    private function verifyPassword($password, $storedPassword, $storedSalt) {
        if ($storedSalt === '') {
            return hash_equals($storedPassword, $password);
        }
        return hash_equals($storedPassword, $this->hashPassword($password, $storedSalt));
    }
    
    public function validateSession() {
        if (isset($_SESSION['eid'])) {
            return $_SESSION['eid'];
        }
        return false;
    }
    
    public function getUserInfo($employeeid) {
        $sql = 'SELECT firstname, lastname, email FROM Employees WHERE employeeid=?;';
        $stmt = $this->db->prepare($sql);
        $stmt->bind_param('i', $employeeid);
        $stmt->execute();
        $result = $stmt->get_result();
        $user = $result->fetch_assoc();
        $stmt->close();
        
        return $user;
    }
}