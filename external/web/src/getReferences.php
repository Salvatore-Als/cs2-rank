<?php

include __DIR__ . '/config.php';

$dbh = getDatabase();
$cache = null;

function getReferences() {
    global $dbh;
    global $page;
    global $cache;
    global $server;

    if ($cache != null) {
        return $cache;
    }

    $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

    $sth = $dbh->prepare('SELECT id, reference, custom_name FROM cs2_rank_references');
    $sth->execute();

    $rows = $sth->fetchAll();
    $cache = $rows;
    return $rows;
}

header('Content-Type: application/json');
echo json_encode([
    'results' => getReferences(),
]);
exit;
