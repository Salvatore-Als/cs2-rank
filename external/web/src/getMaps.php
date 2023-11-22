<?php
    include 'config.php';

    $dbh = new PDO("mysql:host=" . MYSQL_HOST . ";dbname=" . MYSQL_DATABASE, MYSQL_USER, MYSQL_PASSWORD);
    $cache = null;

    function getMaps() {
        global $dbh;

        $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

        $sth = $dbh->prepare('SELECT id, name FROM cs2_rank_maps');
        $sth->execute();

        $rows = $sth->fetchAll();
        return $rows;
    }

    header("Content-Type: application/json");
    echo json_encode(['results' => getMaps()]);
    exit();
?>