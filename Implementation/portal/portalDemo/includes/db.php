<?php
// basic db connection
$servername = "localhost";
$username = "root";
$password = "";
$dbname = "worldrebalance";

try {
    $conn = new PDO("mysql:host=$servername;dbname=$dbname", $username, $password);
    
    
} catch (PDOException $e) {
    echo "Connection failed: " . $e->getMessage();
}
?>