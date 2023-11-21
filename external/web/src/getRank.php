<?php
    include 'config.php';

    $dbh = new PDO("mysql:host=" . MYSQL_HOST . ";dbname=" . MYSQL_DATABASE, MYSQL_USER, MYSQL_PASSWORD);
    $page = isset($_GET['page']) ? intval($_GET['page']) : 1;
    $reference = isset($_GET['reference']) ? strval($_GET['reference']) : null;

    if(!isset($reference) || empty($reference)) {
        echo "Empty reference";
        exit(404);
    }

    $cache = [];

    function getPlayers() {
        global $dbh;
        global $page;
        global $cache;
        global $reference;

        if(in_array($page, $cache, true)) {
            return $cache[$page];
        }

        $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

        $offset = ($page - 1) * PLAYERS_PER_PAGE;
        
        $sth = $dbh->prepare('SELECT * FROM cs2_rank_users WHERE points >= :minPoints AND reference = :reference ORDER BY points DESC LIMIT :offset, :limit');
        $sth->bindValue(':reference', $reference, PDO::PARAM_STR);
        $sth->bindValue(':minPoints', MINIMUM_POINTS, PDO::PARAM_INT);
        $sth->bindValue(':offset', $offset, PDO::PARAM_INT);
        $sth->bindValue(':limit', PLAYERS_PER_PAGE, PDO::PARAM_INT);
        $sth->execute();

        $rows = $sth->fetchAll();
        $players = [];

        $index = 1;
        foreach ($rows as $row) {
            $kills = $row['kill_ct'] + $row['kill_t'];
            $killsCT = floor(calcPercent($row['kill_ct'], $kills));
            $killsT = floor(calcPercent($row['kill_t'], $kills));
            $headshot = floor(calcPercent($row['kill_headshot'], $kills));
            $knife = floor(calcPercent($row['kill_knife'], $kills));
            
            $killMeta = array(
                'total' => $kills,
                'ct' => $killsCT,
                't' => $killsT,
                'headshot' => $headshot,
                'knife' => $knife
            );

            $killassist = $row['killassist_t'] + $row['killassist_ct'];
            $killassistCT = floor(calcPercent($row['killassist_ct'], $killassist));
            $killassistT = floor(calcPercent($row['killassist_t'], $killassist));
        
            $killassistMeta = array(
                'total' => $killassist,
                'ct' => $killassistCT,
                't' => $killassistT
            );

            $death = $row['death_t'] + $row['death_ct'];
            $deathCT = floor(calcPercent($row['death_ct'], $death));
            $deathT = floor(calcPercent($row['death_t'], $death));
            $deathSuicide = floor(calcPercent($row['death_suicide'], $death));

            $deathMeta = array(
                'total' => $death,
                'ct' => $deathCT,
                't' => $deathT,
                'suicide' => $deathSuicide
            );

            $metaData = array(
                'rank' => $index * $page,
                'kill' => $killMeta,
                'death' => $deathMeta,
                'killassist' => $killassistMeta
            );

            $player = array_merge($row , ['metadata' => $metaData]);
            array_push($players, $player);

            $index++;
        }

        array_push($cache, [$page => $players]);
        return $players;
    }

    function getTotalPage() {
        global $dbh;
        global $page;
        global $reference;

        $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

        $offset = ($page - 1) * PLAYERS_PER_PAGE;
            
        $sth = $dbh->prepare('SELECT CEIL(COUNT(*) / :itemsPerPage) as result FROM cs2_rank_users WHERE reference = :reference');
        $sth->bindValue(':reference', $reference, PDO::PARAM_INT);
        $sth->bindValue(':itemsPerPage', PLAYERS_PER_PAGE, PDO::PARAM_INT);
        $sth->execute();

        $result = $sth->fetch(PDO::FETCH_ASSOC);
        return $result['result'];
    }

    function calcPercent($want, $total)
    {
        if ($want <= 0 || $total <= 0)
            return 0.0;

        return ($want / $total) * 100.0;
    }

    header("Content-Type: application/json");
    echo json_encode(['currentPage' => $page, 'totalPage' => getTotalPage(), 'reference' => $reference, 'results' => getPlayers()]);
    exit();
?>