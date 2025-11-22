<?php
    $servername = "localhost";
    $username = "roshn";
    $password = "P@ssword";
    $database = "trial";
    $conn = new mysqli($servername, $username, $password, $database);
    if ($conn->connect_error) {
      die('Connection failed: ' . $conn->connect_error);
    }
?>
