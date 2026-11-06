<?php
class GanttChart {
    private $plannedStart ;
    private $plannedFinish ;
    private $forecastStart ;
    private $forecastFinish ;
    private $actualStart ;
    private $actualFinish ;

    // Signed number of days from $from to $to (DateInterval::days is always absolute)
    private function daysBetween($from, $to) {
        $interval = $from->diff($to);
        return $interval->invert ? -$interval->days : $interval->days;
    }

    private function clampPercent($value) {
        return max(0, min(100, $value));
    }

    private function calculatePosition($date, $minDate, $totalDays) {
        if (!$date) return 0;
        return $this->clampPercent(($this->daysBetween($minDate, $date) / $totalDays) * 100);
    }

    private function calculateWidth($startDate, $endDate, $minDate, $totalDays) {
        if (!$startDate || !$endDate) return 0;
        $left = $this->calculatePosition($startDate, $minDate, $totalDays);
        $right = $this->calculatePosition($endDate, $minDate, $totalDays);
        return max(0, $right - $left);
    }

    private function drawGanttBar($barType, $startDate, $endDate, $minDate, $totalDays){
        //barType:planned/forecast/actual
        $barPosition = $this->calculatePosition($startDate, $minDate, $totalDays);
        $actualEndDate = $endDate ?: new DateTime(); // Use current date if not finished
        $barWidth = $this->calculateWidth($startDate, $actualEndDate, $minDate, $totalDays);

        // Add label for bar type
        $barLabel = ucfirst($barType);

        $html = '';
        $html .= '<div class="chart-row">';
        $html .= '<div class="chart-label">' . $barLabel . '</div>';
        $html .= '<div class="chart-track">';
        $html .= '<div class="chart-bar chart-bar-'.$barType.($endDate ? '' : ' chart-bar-ongoing').'" style="left: '.$barPosition.'%; width: '.$barWidth.'%;"></div>';
        $html .= '</div>';
        $html .= '<div class="chart-value">';
        $html .= $startDate->format('M j') . ' – ';
        $html .= $endDate ? $endDate->format('M j') : 'Ongoing';
        $html .= '</div>';
        $html .= '</div>';
        return $html;
    }

    public function __construct ($plannedStart, $plannedFinish, $forecastStart, $forecastFinish, $actualStart, $actualFinish){
        $this->plannedStart=$plannedStart;
        $this->plannedFinish=$plannedFinish;
        $this->forecastStart=$forecastStart;
        $this->forecastFinish=$forecastFinish;
        $this->actualStart=$actualStart;
        $this->actualFinish=$actualFinish;
    }

    public function draw(){
        $allDates = array_filter([$this->plannedStart, $this->plannedFinish, $this->forecastStart, $this->forecastFinish, $this->actualStart, $this->actualFinish]);
        if (empty($allDates)) {
            return '<p class="text-muted small mb-0">No schedule dates set.</p>';
        }
        // An ongoing actual bar is drawn up to today, so the timeline must include today
        if ($this->actualStart && !$this->actualFinish) {
            $allDates[] = new DateTime();
        }
        $minDate = min($allDates);
        $maxDate = max($allDates);

        // Add some padding to the timeline (clone to avoid modifying original dates)
        $minDate = clone $minDate;
        $maxDate = clone $maxDate;
        // Pad by 7 days or 5% of the span, whichever is larger, so long timelines stay proportionate
        $padDays = max(7, intval(ceil($minDate->diff($maxDate)->days * 0.05)));
        $minDate->modify('-' . $padDays . ' days');
        $maxDate->modify('+' . $padDays . ' days');

        $totalDays = $minDate->diff($maxDate)->days;
        $totalDays = $totalDays > 0 ? $totalDays : 1;
        // Show the year on axis ticks when the timeline spans more than one calendar year
        $tickFormat = $minDate->format('Y') !== $maxDate->format('Y') ? "M j 'y" : 'M j';

        $html = '';
        $html .= '<div class="chart">';
        $html .= '<div class="chart-row chart-axis">';
        $html .= '<div class="chart-label"></div>';
        $html .= '<div class="chart-track">';
        // Add date markers
        $currentDate = clone $minDate;
        while ($currentDate <= $maxDate) {
            $position = $this->calculatePosition($currentDate, $minDate, $totalDays);
            $html .= '<div class="chart-tick" style="left: ' . $position . '%;">';
            $html .= $currentDate->format($tickFormat);
            $html .= '</div>';
            $currentDate->modify('+' . max(1, intval($totalDays / 5)) . ' days');
        }
        $html .= '</div>';
        $html .= '<div class="chart-value"></div>';
        $html .= '</div>';

        if ($this->plannedStart && $this->plannedFinish){
            $html .= $this->drawGanttBar('planned',$this->plannedStart, $this->plannedFinish, $minDate, $totalDays);
        }
        if ($this->forecastStart && $this->forecastFinish){
            $html .= $this->drawGanttBar('forecast',$this->forecastStart, $this->forecastFinish, $minDate, $totalDays);
        }
        if ($this->actualStart){
            $html .= $this->drawGanttBar('actual',$this->actualStart,$this->actualFinish,$minDate, $totalDays);
        }

        // Add today marker
        $today = new DateTime();
        if ($today >= $minDate && $today <= $maxDate){
            $todayPosition = $this->calculatePosition($today, $minDate, $totalDays) / 100;
            $html .= '<div class="chart-today" style="--pos: '.$todayPosition.';">';
            $html .= '<span class="chart-today-label">Today</span>';
            $html .= '</div>';
        }
        $html .= '</div>';

        return $html;
    }
}
?>

