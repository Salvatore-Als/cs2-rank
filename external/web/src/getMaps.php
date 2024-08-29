<?php
include __DIR__ . '/config.php';

$dbh = getDatabase();
$cache = null;

function getMaps() {
    global $dbh;

    $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

    $sth = $dbh->prepare('SELECT id, name FROM cs2_rank_maps');
    $sth->execute();

    return $sth->fetchAll();
}

header('Content-Type: application/json');
echo json_encode([
    'results' => getMaps(),
]);
exit;
