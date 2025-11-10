<?php
header('Content-Type: application/json');
require 'includes/db.php';

$q = trim($_GET['q'] ?? '');
if ($q === '') {
    echo json_encode(['type' => 'none']);
    exit;
}

// 检查是否是玩家
$stmt1 = $conn->prepare("SELECT COUNT(*) FROM player_stats WHERE PlayerName = :q");
$stmt1->execute([':q' => $q]);
$isPlayer = $stmt1->fetchColumn() > 0;

// 检查是否是车辆
$stmt2 = $conn->prepare("SELECT COUNT(*) FROM vehicles WHERE Name = :q");
$stmt2->execute([':q' => $q]);
$isVehicle = $stmt2->fetchColumn() > 0;

if ($isPlayer) {
    echo json_encode(['type' => 'player', 'name' => $q]);
} elseif ($isVehicle) {
    echo json_encode(['type' => 'vehicle', 'name' => $q]);
} else {
    echo json_encode(['type' => 'none']);
}
