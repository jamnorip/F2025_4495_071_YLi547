<?php
header('Content-Type: application/json');
require 'includes/db.php';

// 前端传来的参数（都可选）
$q       = trim($_GET['q'] ?? '');
$country = trim($_GET['country'] ?? '');
$role    = trim($_GET['role'] ?? '');

// 动态拼 WHERE（用预处理防注入）
$where = [];
$params = [];

if ($q !== '') {
  $where[] = "LOWER(Name) LIKE :q";
  $params[':q'] = '%' . strtolower($q) . '%';
}
if ($country !== '') {
  $where[] = "Country = :country";
  $params[':country'] = $country;
}
if ($role !== '') {
  $where[] = "Role = :role";
  $params[':role'] = $role;
}

$whereSql = $where ? ('WHERE ' . implode(' AND ', $where)) : '';

// 查询所有匹配数据（不分页）
$sql = "SELECT Name, Country, Role, MatchesPlayed, Wins, KDRatio,
               ROUND(Wins / NULLIF(MatchesPlayed, 0) * 100, 1) AS WinRate
        FROM vehicles
        $whereSql
        ORDER BY WinRate DESC";

$stmt = $conn->prepare($sql);
foreach ($params as $k => $v) {
  $stmt->bindValue($k, $v);
}
$stmt->execute();

$rows = $stmt->fetchAll(PDO::FETCH_ASSOC);

echo json_encode([
  'rows' => $rows
], JSON_UNESCAPED_UNICODE);
