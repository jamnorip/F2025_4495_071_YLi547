<?php
// leave to Home Page!!! to check if it is name or vehicles or a squad rank
header('Content-Type: application/json');
require 'includes/db.php';

$q = trim($_GET['q'] ?? '');
if ($q === '') {
    echo json_encode(['type' => 'none']);
    exit;
}

// if a player or not
$stmt1 = $conn->prepare("SELECT COUNT(*) FROM player_stats WHERE PlayerName = :q");
$stmt1->execute([':q' => $q]);
$isPlayer = $stmt1->fetchColumn() > 0;

//if a vehicles or not
$stmt2 = $conn->prepare("SELECT COUNT(*) FROM vehicles WHERE Name = :q");
$stmt2->execute([':q' => $q]);
$isVehicle = $stmt2->fetchColumn() > 0;


if ($isPlayer) {
    echo json_encode(['type' => 'player']);
} elseif ($isVehicle) {
    echo json_encode(['type' => 'vehicle']);
} else {
    echo json_encode(['type' => 'none']);
}
