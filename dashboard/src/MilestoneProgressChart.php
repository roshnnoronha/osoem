<?php
// Artefact milestone progress as of today, drawn in the same horizontal bar format as HoursChart.
// Each activity link contributes ratio * (highest milestone step progressratio reached) to the artefact's progress.
//   Planned  - steps whose planned date has passed
//   Forecast - completed steps, plus outstanding steps whose forecast (else planned) date has passed
//   Actual   - steps that have been completed
class MilestoneProgressChart {
    private $links;
    private $stepsByLink;
    private $today;

    // $links: rows with artefactlinkid and ratio; $stepsByLink: artefactlinkid => milestone step rows
    public function __construct(array $links, array $stepsByLink) {
        $this->links = $links;
        $this->stepsByLink = $stepsByLink;
        $this->today = new DateTimeImmutable('today');
    }

    private function stepDate($step, $series) {
        switch ($series) {
            case 'planned':
                $date = $step['planned_date'];
                break;
            case 'forecast':
                $date = $step['completiondate'] ?: ($step['forecast_date'] ?: $step['planned_date']);
                break;
            default:
                $date = $step['completiondate'];
        }
        return $date ? new DateTimeImmutable($date) : null;
    }

    // Percentage of the artefact reached by today for the given series
    private function progressToday($series) {
        $total = 0;
        foreach ($this->links as $link) {
            $linkid = (int)$link['artefactlinkid'];
            $reached = 0;
            foreach ($this->stepsByLink[$linkid] ?? [] as $step) {
                $date = $this->stepDate($step, $series);
                if ($date && $date <= $this->today) {
                    $reached = max($reached, (float)$step['progressratio']);
                }
            }
            $total += (float)$link['ratio'] * $reached;
        }
        return min(100, $total * 100);
    }

    private function drawProgressBar($barType, $percent, $label) {
        $html = '<div class="chart-row">';
        $html .= '<div class="chart-label">' . $label . '</div>';
        $html .= '<div class="chart-track">';
        $html .= '<div class="chart-bar chart-bar-' . $barType . '" style="width: ' . $percent . '%;"></div>';
        $html .= '</div>';
        $html .= '<div class="chart-value">' . number_format($percent, 1) . '%</div>';
        $html .= '</div>';
        return $html;
    }

    public function draw() {
        $html = '<div class="chart" style="--value-w: 70px;">';
        $html .= $this->drawProgressBar('planned', $this->progressToday('planned'), 'Planned');
        $html .= $this->drawProgressBar('forecast', $this->progressToday('forecast'), 'Forecast');
        $html .= $this->drawProgressBar('actual', $this->progressToday('actual'), 'Actual');
        $html .= '</div>';
        return $html;
    }
}
