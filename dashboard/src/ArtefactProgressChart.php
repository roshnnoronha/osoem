<?php
class ArtefactProgressChart {
    private $progressData;

    public function __construct($progressData) {
        $this->progressData = $progressData;
    }

    private function drawProgressBar($activityName, $progress, $ratio, $artefactlinkid) {
        // Calculate scaled progress based on ratio
        $scaledProgress = $progress * $ratio;

        $html = '';
        $html .= '<div class="chart-row">';
        $html .= '<div class="chart-label">';
        $html .= htmlspecialchars($activityName);
        $html .= ' <span class="text-body-tertiary">(' . number_format($ratio * 100, 0) . '%)</span>';
        $html .= '</div>';
        $html .= '<div class="chart-track">';
        $html .= '<div class="chart-bar chart-bar-actual" style="width: ' . $scaledProgress . '%;"></div>';
        $html .= '</div>';
        $html .= '<div class="chart-value d-flex align-items-center justify-content-between">';
        $html .= number_format($progress, 1) . '%';
        $html .= '<a href="artefact.php?artefactlinkid=' . urlencode($artefactlinkid) . '" class="btn btn-sm btn-outline-secondary py-0" title="Edit milestones">';
        $html .= '<i class="bi bi-flag"></i>Milestones';
        $html .= '</a>';
        $html .= '</div>';
        $html .= '</div>';
        return $html;
    }

    private function drawOverallProgressBar($overallProgress) {
        $html = '';
        $html .= '<div class="chart-row chart-total">';
        $html .= '<div class="chart-label">Overall</div>';
        $html .= '<div class="chart-track">';
        $html .= '<div class="chart-bar chart-bar-actual" style="width: ' . $overallProgress . '%;"></div>';
        $html .= '</div>';
        $html .= '<div class="chart-value fw-semibold">' . number_format($overallProgress, 1) . '%</div>';
        $html .= '</div>';
        return $html;
    }

    public function draw() {
        $html = '<div class="chart" style="--label-w: 160px; --value-w: 150px;">';
        // Draw header with percentage markers
        $html .= '<div class="chart-row chart-axis">';
        $html .= '<div class="chart-label"></div>';
        $html .= '<div class="chart-track">';
        for ($i = 0; $i <= 100; $i += 25) {
            $html .= '<div class="chart-tick" style="left: ' . $i . '%;">' . $i . '%</div>';
        }
        $html .= '</div>';
        $html .= '<div class="chart-value"></div>';
        $html .= '</div>';

        // Draw progress bars for each activity
        $overallProgress = 0;
        foreach ($this->progressData as $data) {
            $html .= $this->drawProgressBar($data['activityname'], $data['progress'], $data['ratio'], $data['artefactlinkid']);
            $overallProgress += $data['progress'] * $data['ratio'];
        }

        // Draw overall progress bar
        $html .= $this->drawOverallProgressBar($overallProgress);
        $html .= '</div>';

        return $html;
    }
}
?>
