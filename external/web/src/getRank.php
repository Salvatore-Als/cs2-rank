<?php
    include 'config.php';

    $dbh = new PDO("mysql:host=" . MYSQL_HOST . ";dbname=" . MYSQL_DATABASE, MYSQL_USER, MYSQL_PASSWORD);
    $page = isset($_GET['page']) ? intval($_GET['page']) : 1;
    $reference = isset($_GET['reference']) ? strval($_GET['reference']) : null;
    $map = (isset($_GET['map']) && is_numeric($_GET['map']) && ($_GET['map'] > 0)) ? intval($_GET['map']) : null;

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
        global $map;

        if(in_array($page, $cache, true)) {
            return $cache[$page];
        }

        $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

        $offset = ($page - 1) * PLAYERS_PER_PAGE;
        
        if(!empty($map)) {
            $sth = $dbh->prepare('SELECT * FROM cs2_rank_users WHERE points >= :minPoints AND reference = :reference AND map = :map ORDER BY points DESC LIMIT :offset, :limit');
            $sth->bindValue(':reference', $reference, PDO::PARAM_STR);
            $sth->bindValue(':map', $map, PDO::PARAM_INT);
            $sth->bindValue(':minPoints', MINIMUM_POINTS, PDO::PARAM_INT);
            $sth->bindValue(':offset', $offset, PDO::PARAM_INT);
            $sth->bindValue(':limit', PLAYERS_PER_PAGE, PDO::PARAM_INT);
            $sth->execute();
        } 
        else {
            //$sth = $dbh->prepare('SELECT * FROM cs2_rank_users WHERE points >= :minPoints AND reference = :reference ORDER BY points DESC LIMIT :offset, :limit');

            $sth = $dbh->prepare('SELECT authid, MIN(name) AS name, MAX(lastconnect) AS lastconnect,'.
                'SUM(points) AS points, SUM(death_suicide) AS death_suicide,'.
                'SUM(death_t) AS death_t, SUM(death_ct) AS death_ct, SUM(bomb_planted) AS bomb_planted,'.
                'SUM(bomb_exploded) AS bomb_exploded, SUM(bomb_defused) AS bomb_defused, SUM(kill_knife) AS kill_knife, '.
                'SUM(kill_headshot) AS kill_headshot, SUM(kill_t) AS kill_t, SUM(kill_ct) AS kill_ct, '.
                'SUM(teamkill_t) AS teamkill_t, SUM(teamkill_ct) AS teamkill_ct, SUM(killassist_t) AS killassist_t, '.
                'SUM(killassist_ct) AS killassist_ct FROM cs2_rank_users WHERE reference = :reference '.
                'GROUP BY authid HAVING SUM(points) > :minPoints ORDER BY points DESC LIMIT :offset, :limit;');

            $sth->bindValue(':reference', $reference, PDO::PARAM_STR);
            $sth->bindValue(':minPoints', MINIMUM_POINTS, PDO::PARAM_INT);
            $sth->bindValue(':offset', $offset, PDO::PARAM_INT);
            $sth->bindValue(':limit', PLAYERS_PER_PAGE, PDO::PARAM_INT);
            $sth->execute();
        }

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
        global $map;

        $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

        $offset = ($page - 1) * PLAYERS_PER_PAGE;
            
        if(!empty($map)) {
            $sth = $dbh->prepare('SELECT CEIL(COUNT(*) / :itemsPerPage) as result FROM cs2_rank_users WHERE reference = :reference AND map = :map');
            $sth->bindValue(':map', $map, PDO::PARAM_INT);
            $sth->bindValue(':reference', $reference, PDO::PARAM_INT);
            $sth->bindValue(':itemsPerPage', PLAYERS_PER_PAGE, PDO::PARAM_INT);
            $sth->execute();
        } 
        else {
            $sth = $dbh->prepare('SELECT CEIL(COUNT(*) / :itemsPerPage) as result FROM cs2_rank_users WHERE reference = :reference');
            $sth->bindValue(':reference', $reference, PDO::PARAM_INT);
            $sth->bindValue(':itemsPerPage', PLAYERS_PER_PAGE, PDO::PARAM_INT);
            $sth->execute();
        }

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
    echo json_encode(['currentPage' => $page, 'totalPage' => getTotalPage(), 'reference' => $reference, 'map' => $map, 'results' => getPlayers()]);
    exit();
?>