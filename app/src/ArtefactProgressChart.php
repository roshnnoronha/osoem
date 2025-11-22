<?php
class ArtefactProgressChart {
    private $progressData;

    public function __construct($progressData) {
        $this->progressData = $progressData;
    }

    private function drawProgressBar($activityName, $progress, $ratio, $artefactlinkid) {
        // Calculate scaled progress based on ratio
        $scaledProgress = $progress * $ratio;

        echo '<div class="gantt-row">';
        echo '<div class="gantt-label" style="display: inline-block; width: 150px; vertical-align: top; font-size: 12px;">';
        echo htmlspecialchars($activityName);
        echo ' (' . number_format($ratio * 100, 0) . '%)';
        echo '</div>';
        echo '<div class="gantt-timeline" style="position: relative;">';
        echo '<div class="gantt-bar gantt-bar-actual" style="left: 0%; width: ' . $scaledProgress . '%;">';
        echo '<span class="gantt-bar-span">';
        echo number_format($progress, 1) . '%';
        echo '</span>';
        echo '</div>';
        echo '<button type="button" class="btn btn-primary btn-xs" ';
        echo 'onclick="navigateToMilestones(' . $artefactlinkid . ')" ';
        echo 'style="position: absolute; right: 5px; top: 50%; transform: translateY(-50%);" ';
        echo 'title="Edit milestones">';
        echo '<span class="glyphicon glyphicon-edit"></span> Edit';
        echo '</button>';
        echo '</div>';
        echo '</div>';
    }

    private function drawOverallProgressBar($overallProgress) {
        echo '<div class="gantt-row" style="border-top: 2px solid #333; margin-top: 10px; padding-top: 10px;">';
        echo '<div class="gantt-label" style="display: inline-block; width: 150px; vertical-align: top; font-weight: bold;">Overall Progress</div>';
        echo '<div class="gantt-timeline">';
        echo '<div class="gantt-bar gantt-bar-actual" style="left: 0%; width: ' . $overallProgress . '%;">';
        echo '<span class="gantt-bar-span">';
        echo number_format($overallProgress, 1) . '%';
        echo '</span>';
        echo '</div>';
        echo '</div>';
        echo '</div>';
    }

    public function draw() {
        // Draw header with percentage markers
        echo '<div class="gantt-header">';
        echo '<div class="gantt-timeline-bar">';
        for ($i = 0; $i <= 100; $i += 20) {
            echo '<div class="timeline-marker" style="left: ' . $i . '%;">';
            echo $i . '%';
            echo '</div>';
        }
        echo '</div>';
        echo '</div>';

        // Draw progress bars for each activity
        $overallProgress = 0;
        foreach ($this->progressData as $data) {
            $this->drawProgressBar($data['activityname'], $data['progress'], $data['ratio'], $data['artefactlinkid']);
            $overallProgress += $data['progress'] * $data['ratio'];
        }

        // Draw overall progress bar
        $this->drawOverallProgressBar($overallProgress);
    }
}
?>
