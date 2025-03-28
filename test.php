<?php
echo "<html><head><title>CGI Test</title></head><body>";
echo "<h1>CGI Test Page</h1>";

// Handle GET request
if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    echo "<h2>GET Request</h2>";
    echo "Query string: " . $_SERVER['QUERY_STRING'] . "<br>";
    if (!empty($_GET)) {
        echo "GET parameters:<br>";
        foreach ($_GET as $key => $value) {
            echo "$key: $value<br>";
        }
    }
}

// Handle POST request
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    echo "<h2>POST Request</h2>";
    echo "POST data:<br>";
    foreach ($_POST as $key => $value) {
        echo "$key: $value<br>";
    }
}

// Add a form for POST testing
echo "<h2>Test POST Form</h2>";
echo "<form method='POST'>";
echo "Name: <input type='text' name='name'><br>";
echo "Age: <input type='number' name='age'><br>";
echo "<input type='submit' value='Submit'>";
echo "</form>";

echo "</body></html>";
?> 