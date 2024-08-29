<?php

define("STEAMAPI_KEY", ""); // Your Steam WebAPI-Key found at https://steamcommunity.com/dev/apikey

define("MYSQL_HOST", "localhost"); // Mysql host
define("MYSQL_PASSWORD", "8io/PAz838st03"); // Mysql password
define("MYSQL_DATABASE", "cs2"); // Mysql database
define("MYSQL_USER", "root"); // Mysql user

define("PLAYERS_PER_PAGE", "50");
define("MINIMUM_POINTS", 10); // Minimum points to be on the rank, should be the same config as your plugin config

// IGNORE EVERYTHING BELOW THIS LINE
function getDatabase() {
    global $dbh;

    if ($dbh == null) {
        $dbh = new PDO("mysql:host=" . MYSQL_HOST . ";dbname=" . MYSQL_DATABASE, MYSQL_USER, MYSQL_PASSWORD);
    }

    return $dbh;
}
