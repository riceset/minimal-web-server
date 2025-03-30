<?php
echo "<html><head><title>Environment POST Test</title></head><body>";
echo "<h1>Environment POST Test</h1>";

// Output environment variables
echo "<h2>Environment Variables:</h2>";
echo "<pre>";
foreach ($_SERVER as $key => $value) {
    echo "$key: $value\n";
}
echo "</pre>";

// Check for HTTP_RAW_POST_DATA specifically
echo "<h2>Raw POST Data:</h2>";
echo "<pre>";
if (isset($_SERVER['HTTP_RAW_POST_DATA'])) {
    echo $_SERVER['HTTP_RAW_POST_DATA'];
} else {
    echo "HTTP_RAW_POST_DATA not set";
}
echo "</pre>";

// Parse the raw post data manually
echo "<h2>Parsed POST Data:</h2>";
echo "<pre>";
if (isset($_SERVER['HTTP_RAW_POST_DATA'])) {
    parse_str($_SERVER['HTTP_RAW_POST_DATA'], $parsed_data);
    print_r($parsed_data);
} else {
    echo "No data to parse";
}
echo "</pre>";

// Also check php://input for comparison
echo "<h2>php://input content:</h2>";
echo "<pre>";
$input_data = file_get_contents('php://input');
echo $input_data ?: "No data in php://input";
echo "</pre>";

echo "</body></html>";
?> 