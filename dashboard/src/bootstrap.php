<?php

require_once __DIR__ . '/autoload.php';

session_start();

$authService = new AuthService();
$eid = $authService->validateSession();

if (!$eid) {
    header('Location: index.php');
    exit();
}
