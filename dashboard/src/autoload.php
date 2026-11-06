<?php

$envFile = __DIR__ . '/../.env';
if (file_exists($envFile)) {
    foreach (file($envFile, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES) as $line) {
        if ($line[0] === '#' || strpos($line, '=') === false) {
            continue;
        }
        [$name, $value] = explode('=', $line, 2);
        $name = trim($name);
        if (getenv($name) === false) {
            putenv($name . '=' . trim($value));
        }
    }
}

spl_autoload_register(function ($class_name) {
    $base_dir = __DIR__ . '/';
    
    $file = $base_dir . $class_name . '.php';
    if (file_exists($file)) {
        require_once $file;
        return;
    }
    
    $file = $base_dir . 'Services/' . $class_name . '.php';
    if (file_exists($file)) {
        require_once $file;
        return;
    }
});