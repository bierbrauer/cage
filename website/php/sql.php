<?php

$hostname = "localhost";
$username = "root";
$password = "root";
$database = "cage";

$connection = mysql_connect(
	$hostname,
	$username,
	$password
);

if (!$connection) {
  die("ERROR connecting to DB: " . mysql_error());
}

$database = mysql_select_db($database);

if (!$database) {
  die("ERROR selecting DB: " . mysql_error());
}

$query = mysql_query("SELECT * FROM trial_data");

$trial_data = [];

while ($row = mysql_fetch_assoc($query)) {
    array_push($trial_data, $row);
}

die(json_encode($trial_data));

?>