<?php

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