<?php
    include 'config.php';

    $dbh = new PDO("mysql:host=" . MYSQL_HOST . ";dbname=" . MYSQL_DATABASE, MYSQL_USER, MYSQL_PASSWORD);
    $cache = null;

    function getServers() {
        global $dbh;
        global $page;
        global $cache;
        global $server;

        if($cache != null) {
            return $cache;
        }

        $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

        $sth = $dbh->prepare('SELECT id, reference, custom_name FROM verygames_rank_servers');
        $sth->execute();

        $rows = $sth->fetchAll();
        $cache = $rows;
        return $rows;
    }

    header("Content-Type: application/json");
    echo json_encode(['results' => getServers()]);
    exit();
?>