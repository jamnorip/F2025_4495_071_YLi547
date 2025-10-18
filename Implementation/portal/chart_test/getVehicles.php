<?php
header('Content-Type: application/json');

//连接数据库
$host = "localhost";
$user = "root";
$pass = "";
$dbname = "test";  
$conn = new mysqli($host, $user, $pass, $dbname);
if ($conn->connect_error) {
    die(json_encode(["error" => "DB connection failed: " . $conn->connect_error]));
}

//查询数据
$sql = "SELECT name, count FROM vehicles";
$result = $conn->query($sql);   //获得查询结果

$labels = [];
$data = [];

while ($row = $result->fetch_assoc()) {
    $labels[] = $row["name"];    //每一行的name放入到$labels[]数组中
    $data[] = (int)$row["count"];   //每一行的count放入到$data[]数组中
}

//输出 JSON 格式
echo json_encode([      //使用的是关联数组，也就是labels对应$labels,data对应$data
    "labels" => $labels, 
    "data" => $data
]);

$conn->close();
?>
