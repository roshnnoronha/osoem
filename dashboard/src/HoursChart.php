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
        $html = '';
        $html .= '<div class="chart-row">';
        $html .= '<div class="chart-label">' . $label . '</div>';
        $html .= '<div class="chart-track">';
        $html .= '<div class="chart-bar chart-bar-'.$barType.'" style="width: '.($hours / $maxHours * 100).'%;"></div>';
        $html .= '</div>';
        $html .= '<div class="chart-value">' . number_format($hours, 1) . ' h</div>';
        $html .= '</div>';
        return $html;
    }
    public function draw(){
        $maxHours = max($this->plannedHours, $this->forecastHours, $this->consumedHours);
        $maxHours = $maxHours > 0 ? $maxHours : 1; // Prevent division by zero
        $html = '<div class="chart" style="--value-w: 70px;">';
        $html .= $this->drawHourBar('planned',$this->plannedHours,$maxHours,'Planned');
        $html .= $this->drawHourBar('forecast',$this->forecastHours,$maxHours,'Forecast');
        $html .= $this->drawHourBar($this->consumedHours > $this->plannedHours? 'consumed-over' : 'consumed-normal',$this->consumedHours,$maxHours,'Actual');
        $html .= '</div>';
        return $html;
    }
}
?>
