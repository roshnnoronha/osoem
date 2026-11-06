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

    private function getStatusIcon($taskId,$tasks) {
        switch($tasks[$taskId]['status']) {
            case TaskStatus::COMPLETE: return '<i class="bi bi-check-circle-fill text-success" title="Complete"></i>';
            case TaskStatus::IN_PROGRESS: return '<i class="bi bi-circle-half text-primary" title="In progress"></i>';
            case TaskStatus::NOT_STARTED: return '<i class="bi bi-circle text-body-tertiary" title="Not started"></i>';
            default: return '<i class="bi bi-circle text-body-tertiary"></i>';
        }
    }

    private function printTaskTree($parentId, $tasks) {
        $html = '';
        foreach ($this->getChildren($parentId, $tasks) as $task) {
            $hours = isset($task['hours']) ? $task['hours'] : 0;

            $html .= '<li>';
            $html .= '<div class="task-tree-item">';
            $html .= $this->getStatusIcon($task['taskid'], $tasks);
            $html .= '<a class="task-name" href="tasks.php?taskid=' . urlencode($task['taskid']) . '">' . htmlspecialchars($task['taskname']) . '</a>';
            $html .= '<span class="task-hours">' . htmlspecialchars($hours) . ' h</span>';
            $html .= '</div>';

            // Recursively print children
            $children = $this->printTaskTree($task['taskid'], $tasks);
            if ($children !== '') {
                $html .= '<ul>' . $children . '</ul>';
            }
            $html .= '</li>' . "\n";
        }
        return $html;
    }

    public function draw(){
        $items = $this->printTaskTree(null, $this->tasks);
        if ($items === '') {
            return '<div class="empty-state"><i class="bi bi-diagram-3"></i>No tasks have been added yet.</div>';
        }
        $html = '';
        $html .= '<ul class="task-tree" aria-label="Tasks for ' . htmlspecialchars($this->activityname) . '">';
        $html .= $items;
        $html .= '</ul>';
        $html .= '<div class="task-tree-legend mt-3 pt-3 border-top">';
        $html .= '<span><i class="bi bi-check-circle-fill text-success"></i> Complete</span>';
        $html .= '<span><i class="bi bi-circle-half text-primary"></i> In progress</span>';
        $html .= '<span><i class="bi bi-circle text-body-tertiary"></i> Not started</span>';
        $html .= '</div>';
        return $html;
    }
}
?>
