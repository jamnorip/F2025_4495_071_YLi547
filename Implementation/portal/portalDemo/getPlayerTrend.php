<?php
header('Content-Type: application/json');  //return type


$player = $_GET['player'] ?? 'HaoSUO';


$mode = $_GET['mode'] ?? '';//useless!!

// the basic value
$baseValue = [
    'Realistic' => ['HaoSUO' => 1.3, 'NoobMaster' => 1.1, 'AcePilot' => 1.5],
    ];

$labels = [];
$values = [];

$base = $baseValue[$mode][$player] ?? 1.3;

for ($i = 30; $i >= 0; $i--) {
    $labels[] = date('m-d', strtotime("-$i days"));
    // by time to generate mock data
    $values[] = round($base + sin($i / 5) * 0.15 + (mt_rand(-10, 10) / 100), 2);
}

//send to frontend
echo json_encode([
    "player" => $player,
    "mode" => $mode,
    "labels" => $labels,
    "values" => $values
]);
