<?php
echo "<html><head><title>CGI Test</title></head><body>";
echo "<h1>CGI Test Page</h1>";
echo "<h2>POST Data:</h2>";
echo "<pre>";
print_r($_POST);
echo "</pre>";
echo "<h2>Raw POST Data:</h2>";
echo "<pre>";
echo file_get_contents('php://input');
echo "</pre>";
echo "</body></html>";