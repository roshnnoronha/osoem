<?php

class Database {
    private $connection;
    private static $instance = null;
    
    private function __construct() {
        $servername = getenv('DB_HOST') ?: 'localhost';
        $username = getenv('DB_USERNAME');
        $password = getenv('DB_PASSWORD');
        $database = getenv('DB_DATABASE') ?: 'osoem_database';
        if ($username === false || $password === false) {
            die('Database credentials not configured. Set DB_USERNAME and DB_PASSWORD environment variables.');
        }

        $this->connection = new mysqli($servername, $username, $password, $database);
        
        if ($this->connection->connect_error) {
            die('Connection failed: ' . $this->connection->connect_error);
        }
    }
    
    public static function getInstance() {
        if (self::$instance === null) {
            self::$instance = new self();
        }
        return self::$instance;
    }
    
    public function getConnection() {
        return $this->connection;
    }
    
    public function prepare($sql) {
        return $this->connection->prepare($sql);
    }
    
    public function close() {
        if ($this->connection) {
            $this->connection->close();
        }
    }
    
    private function __clone() {}
    
    public function __wakeup() {
        throw new Exception("Cannot unserialize singleton");
    }
}
