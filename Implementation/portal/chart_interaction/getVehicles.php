<?php
header('Content-Type: application/json');
$labels = ["APC", "Tank", "Jeep", "Artillery", "Bike", "Drone"];
$data = array_map(function () {
    return rand(0, 30);
}, $labels);
echo json_encode(["labels" => $labels, "data" => $data], JSON_UNESCAPED_UNICODE);



// <?php  链接数据库的
// header('Content-Type: application/json');

// $host = "localhost";
// $user = "root";
// $pass = "";
// $db   = "test";    // 改成你的数据库名
// $conn = new mysqli($host, $user, $pass, $db);
// if ($conn->connect_error) {
//   http_response_code(500);
//   echo json_encode(["error" => "DB connection failed"]); exit;
// }

// $sql = "SELECT name, count FROM vehicles";
// $res = $conn->query($sql);
// $labels = []; $data = [];
// while ($row = $res->fetch_assoc()) {
//   $labels[] = $row["name"];
//   $data[]   = (int)$row["count"];
// }
// echo json_encode(["labels"=>$labels,"data"=>$data], JSON_UNESCAPED_UNICODE);
// $conn->close();
