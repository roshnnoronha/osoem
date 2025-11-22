<?php
class GanttChart {
    private $plannedStart ;
    private $plannedFinish ;
    private $forecastStart ;
    private $forecastFinish ;
    private $actualStart ;
    private $actualFinish ;

    private function calculatePosition($date, $minDate, $totalDays) {
        if (!$date) return 0;
        $daysDiff = $minDate->diff($date)->days;
        return ($daysDiff / $totalDays) * 100;
    }

    private function calculateWidth($startDate, $endDate, $minDate, $totalDays) {
        if (!$startDate || !$endDate) return 0;
        $duration = $startDate->diff($endDate)->days;
        return ($duration / $totalDays) * 100;
    }

    private function drawGanttBar($barType, $startDate, $endDate, $minDate, $totalDays){
        //barType:planned/forecast/actual
        $barPosition = $this->calculatePosition($startDate, $minDate, $totalDays);
        $actualEndDate = $endDate ?: new DateTime(); // Use current date if not finished
        $barWidth = $this->calculateWidth($startDate, $actualEndDate, $minDate, $totalDays);
        
        // Add label for bar type
        $barLabel = ucfirst($barType);
        
        echo '<div class="gantt-row">';
        echo '<div class="gantt-label" style="display: inline-block; width: 80px; vertical-align: top;">' . $barLabel . '</div>';
        echo '<div class="gantt-timeline">';
        echo '<div class="gantt-bar gantt-bar-'.$barType.'" style="left: '.$barPosition.'%; width: '.$barWidth.'%;">';
        echo '<span class="gantt-bar-span">';
        echo $startDate->format('M j') . ' - ';
        echo $endDate ? $endDate->format('M j') : 'Ongoing';
        echo '</span>';
        echo '</div>';
        echo '</div>';
        echo '</div>';
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
        $minDate = min($allDates);
        $maxDate = max($allDates);

        // Add some padding to the timeline (clone to avoid modifying original dates)
        $minDate = clone $minDate;
        $maxDate = clone $maxDate;
        $minDate->modify('-7 days');
        $maxDate->modify('+7 days');

        $totalDays = $minDate->diff($maxDate)->days;
        $totalDays = $totalDays > 0 ? $totalDays : 1;
        
        echo '<div class="gantt-header">';
        echo '<div class="gantt-timeline-bar">';
        // Add month markers
        $currentDate = clone $minDate;
        while ($currentDate <= $maxDate) {
            $position = $this->calculatePosition($currentDate, $minDate, $totalDays);
            echo '<div class="timeline-marker" style="left: ' . $position . '%;">';
            echo $currentDate->format('M j');
            echo '</div>';
            $currentDate->modify('+' . max(1, intval($totalDays / 8)) . ' days');
        }
        echo '</div>';
        echo '</div>';

        if ($this->plannedStart && $this->plannedFinish){
            $this->drawGanttBar('planned',$this->plannedStart, $this->plannedFinish, $minDate, $totalDays);
        } 
        if ($this->forecastStart && $this->forecastFinish){
            $this->drawGanttBar('forecast',$this->forecastStart, $this->forecastFinish, $minDate, $totalDays);
        }
        if ($this->actualStart){
            $this->drawGanttBar('actual',$this->actualStart,$this->actualFinish,$minDate, $totalDays);
        }
        
        // Add today marker
        $today = new DateTime();
        if ($today >= $minDate && $today <= $maxDate){
            $todayPosition = $this->calculatePosition($today, $minDate, $totalDays);
            echo '<div class="today-marker-container">';
            echo '<div style="margin-left: 80px;">'; // Align with timeline bars
            echo '<div class="today-marker-line" style="left:'.$todayPosition.'%; position: relative; width: 2px; height: 25px; background-color: #dc3545; margin-top: -25px;">';
            echo '<div class="today-marker-label" style="position: absolute; top: -20px; left: -15px; background-color: #dc3545; color: white; padding: 2px 5px; border-radius: 3px; font-size: 10px;">';
            echo 'Today';
            echo '</div>';
            echo '</div>';
            echo '</div>';
            echo '</div>';
        }
        
    }
}
?>

