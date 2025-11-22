<?php

class HoursChart {
    private $plannedHours;
    private $forecastHours;
    private $consumedHours;

    public function __construct($plannedHours,$forecastHours,$consumedHours){
        $this->plannedHours = $plannedHours;
        $this->forecastHours = $forecastHours;
        $this->consumedHours = $consumedHours;
    }
    private function drawHourBar($barType,$hours,$maxHours,$label){
        echo '<div class="chart-row">';
        echo '<div class="chart-label" style="display: inline-block; width: 80px; vertical-align: top;">' . $label . ': </div>';
        echo '<div class="chart-bar-container">';
        echo '<div class="chart-bar chart-hours-bar chart-hours-'.$barType.'" style="width: '.($hours / $maxHours * 100).'%;">';
        echo '<span class="chart-value chart-hours-value">';
        echo number_format($hours, 1).'h';
        echo '</span>';
        echo '</div>';
        echo '</div>';
        echo '</div>';
    } 
    public function draw(){
        $maxHours = max($this->plannedHours, $this->forecastHours, $this->consumedHours);
        $maxHours = $maxHours > 0 ? $maxHours : 1; // Prevent division by zero
        $this->drawHourBar('planned',$this->plannedHours,$maxHours,'Planned');
        $this->drawHourBar('forecast',$this->forecastHours,$maxHours,'Forecast');
        $this->drawHourBar($this->consumedHours > $this->plannedHours? 'consumed-over' : 'consumed-normal',$this->consumedHours,$maxHours,'Actual');
    }
}
?>
