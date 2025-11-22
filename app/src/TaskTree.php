<?php
class TaskTree {
    private $tasks;
    private $activityname;

    public function __construct($tasks,$activityname){
        $this->tasks = $tasks;
        $this->activityname = $activityname;
    }

    private function getChildren($parentId, $tasks) {
        $children = [];
        foreach ($tasks as $task) {
            if ($task['parenttaskid'] == $parentId) {
                $children[] = $task;
            }
        }
        return $children;
    }
    
    private function isLastChild($taskId, $parentId, $tasks) {
        $children = $this->getChildren($parentId, $tasks);
        $lastChild = end($children);
        return $lastChild && $lastChild['taskid'] == $taskId;
    }
    
    private function getStatusSymbol($taskId,$tasks) {
        switch($tasks[$taskId]['status']) {
            case 0: return '●'; // Completed
            case 1: return '◔'; // In Progress  
            case 2: return '○'; // Not Started
            default: return '○';
        }
    }
    
    private function printTaskTree($parentId, $tasks, $prefix = '', $isLast = true) {
        $children = $this->getChildren($parentId, $tasks);
        
        foreach ($children as $index => $task) {
            $isLastChild = ($index === count($children) - 1);
            $statusSymbol = $this->getStatusSymbol($task['taskid'],$tasks);
            
            // Determine the connector symbol
            if ($parentId == 0 || $parentId === null) {
                // Root level tasks
                if ($isLastChild && count($children) > 1) {
                    $connector = '└──';
                    $nextPrefix = $prefix . '   ';
                } else {
                    $connector = '├──';
                    $nextPrefix = $prefix . '│  ';
                }
            } else {
                // Child tasks
                if ($isLastChild) {
                    $connector = '└──';
                    $nextPrefix = $prefix . '   ';
                } else {
                    $connector = '├──';
                    $nextPrefix = $prefix . '│  ';
                }
            }
            
            // Check if this task has children to determine if it needs a branch symbol
            $hasChildren = !empty($this->getChildren($task['taskid'], $tasks));
            if ($hasChildren && $parentId != 0 && $parentId !== null) {
                $connector = str_replace('──', '─┬', $connector);
            }
            
            // Print the task with hours
            $hours = isset($task['hours']) ? $task['hours'] : 0;
            $hoursDisplay = $hours > 0 ? " ({$hours}h)" : " (0h)";
            echo '<div style="font-family: monospace; ">';
            echo htmlspecialchars($prefix . $connector . $statusSymbol . ' ' . $task['taskname'] . $hoursDisplay);
            echo '</div>' . "\n";
            
            // Recursively print children
            if ($hasChildren) {
                $this->printTaskTree($task['taskid'], $tasks, $nextPrefix, $isLastChild);
            }
        }
    }
    
    public function draw(){
        echo '<div style="font-family: monospace;">';
        echo '&lt;'.$this->activityname.'&gt;';
        $this->printTaskTree(null, $this->tasks);
        echo '</div>';
    }
}
?>
