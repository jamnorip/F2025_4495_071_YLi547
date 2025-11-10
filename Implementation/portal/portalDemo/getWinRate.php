<?php
header('Content-Type: application/json');
require 'includes/db.php';


$player = $_GET['player'] ?? 'HaoSUO';  //to get player_name


$sql = "SELECT WinRate FROM player_stats 
    WHERE PlayerName = :player ";
$stmt = $conn->prepare($sql);
$stmt->execute([':player' => $player]);
$row = $stmt->fetch(PDO::FETCH_ASSOC);

$out = [];
if ($row && $row['WinRate'] !== null) {
    $base = (float)$row['WinRate'];

    
    if ($base <= 1.0) {
        $base *= 100.0;
    }

    
    $clamp = function($v) { return max(0, min(100, round($v, 1))); };

    
    $out = [
        ["mode" => "Main Battle Tank", "win_rate" => $clamp($base)],
        ["mode" => "Heavy Tank",       "win_rate" => $clamp($base - 2)],
        ["mode" => "Medium Tank",      "win_rate" => $clamp($base - 3)],
        ["mode" => "Light Tank",       "win_rate" => $clamp($base - 4)],
    ];
}

echo json_encode($out, JSON_UNESCAPED_UNICODE);
