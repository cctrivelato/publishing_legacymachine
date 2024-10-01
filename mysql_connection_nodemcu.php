<?php
$servername = "localhost";
$username = "root";
$password = "******";
$dbname = "database";

// Create connection to MySQL
$conn = new mysqli($servername, $username, $password, $dbname);

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Check if status parameter is set
if (isset($_GET['status'])) {
    $status = $_GET['status'];

    // SQL to insert LED status
    $sql = "INSERT INTO test.led_status (status) VALUES ('$status')";

    if ($conn->query($sql) === TRUE) {
        echo "New record created successfully";
    } else {
        echo "Error: " . $sql . "<br>" . $conn->error;
    }
} else {
    echo "No status provided";
}

$conn->close();
?>
