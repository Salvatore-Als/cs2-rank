<?php
    include 'config.php';

    $dbh = new PDO("mysql:host=" . MYSQL_HOST . ";dbname=" . MYSQL_DATABASE, MYSQL_USER, MYSQL_PASSWORD);

    $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

    $sth = $dbh->prepare('SELECT * FROM verygames_rank WHERE points >= :minPoints ORDER BY points DESC;');
    $sth->bindValue(':minPoints', MINIMUM_POINTS, PDO::PARAM_INT);
    $sth->execute();

    $rows = $sth->fetchAll();

    $index = 1;
    foreach ($rows as &$row) {
        $row['metadata']['rank'] = $index;

        $kills = $row['kill_ct'] + $row['kill_t'];
        $killsCT = floor(calcPercent($row['kill_ct'], $kills));
        $killsT = floor(calcPercent($row['kill_t'], $kills));
        $headshot = floor(calcPercent($row['kill_headshot'], $kills));
        $knife = floor(calcPercent($row['kill_knife'], $kills));
    
        $row['metadata']['kill'] = array(
            'total' => $kills,
            'ct' => $killsCT,
            't' => $killsT,
            'headshot' => $headshot,
            'knife' => $knife
        );

        $killassist = $row['killassist_t'] + $row['killassist_ct'];
        $killassistCT = floor(calcPercent($row['killassist_ct'], $killassist));
        $killassistT = floor(calcPercent($row['killassist_t'], $killassist));
    
        $row['metadata']['killassist'] = array(
            'total' => $killassist,
            'ct' => $killassistCT,
            't' => $killassistT
        );

        $death = $row['death_t'] + $row['death_ct'];
        $deathCT = floor(calcPercent($row['death_ct'], $death));
        $deathT = floor(calcPercent($row['death_t'], $death));
        $deathSuicide = floor(calcPercent($row['death_suicide'], $death));

        $row['metadata']['death'] = array(
            'total' => $death,
            'ct' => $deathCT,
            't' => $deathT,
            'suicide' => $deathSuicide
        );

        $row['metadata']['picture'] = getAvatar($row['authid']);

        $index++;
    }

    function calcPercent($want, $total)
    {
        if ($want <= 0 || $total <= 0)
            return 0.0;

        return ($want / $total) * 100.0;
    }

    function getAvatar($steam64){

        $url = file_get_contents("https://api.steampowered.com/ISteamUser/GetPlayerSummaries/v0002/?key=".STEAMAPI_KEY."&steamids=".$steam64); 
        $content = json_decode($url, true);
   
        $avatar = $content['response']['players'][0]['avatarmedium'];
        return is_null($avatar) || $avatar == "" ? DEFAULT_STEAM_PICTURE : $avatar;
    }
?>
<!doctype html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link href="style.css" rel="stylesheet">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.2/css/all.min.css" integrity="sha512-z3gLpd7yknf1YoNbCzqRKc4qyor8gaKU1qmn+CShxbuBusANI9QpRohGBreCFkKxLhei6S9CQXFEbbKuqLg0DA==" crossorigin="anonymous" referrerpolicy="no-referrer" />
</head>
<body class="bg-gray-100 dark:bg-gray-900">
    <section class="m-5">
        <div class="flex mb-4 ">
            <h1 class="mb-4 text-3xl font-extrabold tracking-tight text-gray-900 dark:text-white">
                <?php echo TITLE ?>
            </h1>
            <div class="cursor-pointer ml-auto text-3xl font-extrabold tracking-tight text-gray-900 dark:text-white">
                <i id="theme-icon" onclick="changeTheme()"></i>
            </div>
        </div>
        <div class="overflow-hidden rounded-lg shadow-md dark:bg-gray-800 dark:text-white">
            <table class="pt-5 w-full border-collapse dark:bg-gray-800 text-left text-sm text-gray-500 dark:text-white">
                <thead class="bg-gray-50 dark:bg-gray-700 dark:text-gray-400">
                    <tr>
                        <th scope="col" class="px-6 py-4 font-medium text-gray-900 dark:text-white">
                            <i class="fa-solid fa-ranking-star"></i>
                        </th>
                        <th scope="col" class="px-6 py-4 font-medium text-gray-900 dark:text-white">Name</th>
                        <th scope="col" class="px-6 py-4 font-medium text-gray-900 dark:text-white">Points</th>
                        <th scope="col" class="px-6 py-4 font-medium text-gray-900 dark:text-white">Kills</th>
                        <th scope="col" class="px-6 py-4 font-medium text-gray-900 dark:text-white">Assist</th>
                        <th scope="col" class="px-6 py-4 font-medium text-gray-900 dark:text-white">Death</th>
                        <th scope="col" class="px-6 py-4 font-medium text-gray-900 dark:text-white">Bomb</th>
                    </tr>
                </thead>
                <tbody>
                    <?php foreach($rows as $row) { ?>
                        <tr class="bg-white hover:bg-gray-50 dark:bg-gray-800 dark:border-gray-700 dark:hover:bg-gray-600">
                            <td class="px-6 py-4 font-normal text-gray-900 dark:text-white">
                                <?php echo $row['metadata']['rank'];?>
                            </td>
                            <td class="px-6 py-4 font-normal text-gray-900 dark:text-white">
                                <div class="flex gap-2">
                                    <div class="relative h-10 w-10">
                                        <img class="h-full w-full rounded-full object-cover object-center" loading="lazy" src="<?php echo $row['metadata']['picture']?>" alt="" />
                                    </div>

                                    <div class="text-sm">
                                        <div class="font-medium dark:text-gray-300 text-gray-700">
                                            <?php echo $row['name']; ?>
                                        </div>
                                        <a href="https://steamcommunity.com/profiles/<?php echo $row['authid']; ?>" class="text-gray-400">
                                            <?php echo $row['authid']; ?>
                                        </a>
                                    </div>
                                </div>
                            </td>
                            <td class="px-6 py-4">
                                <span class="font-medium dark:text-gray-300 text-gray-700">
                                    <?php echo $row['points']; ?>
                                </span>
                            </td>
                            <td class="px-6 py-4">
                                <div class="flex flex-col gap-1">
                                    <div class="flex gap-2">
                                        <span class="inline-flex items-center font-medium dark:text-gray-300 text-gray-700">
                                            <?php echo $row['metadata']['kill']['total']; ?>
                                        </span>
                                        <span class="inline-flex items-center rounded-md dark:bg-red-700 bg-red-50 px-2 py-1 text-xs font-medium dark:text-red-50 text-red-700 ring-1 ring-inset dark:ring-red-600/10 ring-red-600/10">
                                            <?php echo $row['metadata']['kill']['t']; ?>% T
                                        </span>
                                        <span class="inline-flex items-center rounded-md dark:bg-blue-700 bg-blue-50 px-2 py-1 text-xs font-medium dark:text-blue-50 text-blue-700 ring-1 ring-inset dark:ring-blue-700/10 ring-blue-700/10">
                                            <?php echo $row['metadata']['kill']['ct']; ?>% CT
                                        </span>
                                    </div>
                                    <div class="flex gap-2">
                                        <span class="inline-flex items-center rounded-md dark:bg-gray-700 bg-gray-50 px-2 py-1 text-xs font-medium dark:text-gray-50 text-gray-600 ring-1 ring-inset dark:ring-gray-500/10 ring-gray-500/10">
                                            <?php echo $row['metadata']['kill']['headshot']; ?>% Headshot
                                        </span>
                                        <span class="inline-flex items-center rounded-md dark:bg-gray-700 bg-gray-50 px-2 py-1 text-xs font-medium dark:text-gray-50 text-gray-600 ring-1 ring-inset dark:ring-gray-500/10 ring-gray-500/10">
                                            <?php echo $row['metadata']['kill']['knife']; ?>% Knife
                                        </span>
                                    </div>
                                </div>
                            </td>
                            <td class="px-6 py-4 flex gap-2">
                                <span class="inline-flex items-center font-medium dark:text-gray-300 text-gray-700">
                                    <?php echo $row['metadata']['killassist']['total']; ?>
                                </span>
                                <span class="inline-flex items-center rounded-md dark:bg-red-700 bg-red-50 px-2 py-1 text-xs font-medium dark:text-red-50 text-red-700 ring-1 ring-inset dark:ring-red-600/10 ring-red-600/10"">
                                    <?php echo $row['metadata']['killassist']['t']; ?>% T
                                </span>
                                <span class="inline-flex items-center rounded-md dark:bg-blue-700 bg-blue-50 px-2 py-1 text-xs font-medium dark:text-blue-50 text-blue-700 ring-1 ring-inset dark:ring-blue-700/10 ring-blue-700/10">
                                    <?php echo $row['metadata']['killassist']['ct']; ?>% CT
                                </span>
                            </td>
                            <td class="px-6 py-4">
                                <div class="flex flex-col gap-1">
                                    <div class="flex gap-1">
                                        <span class="inline-flex items-center font-medium dark:text-gray-300 text-gray-700">
                                            <?php echo $row['metadata']['death']['total']; ?>
                                        </span>
                                        <span class="inline-flex items-center rounded-md dark:bg-red-700 bg-red-50 px-2 py-1 text-xs font-medium dark:text-red-50 text-red-700 ring-1 ring-inset dark:ring-red-600/10 ring-red-600/10"">
                                            <?php echo $row['metadata']['death']['t']; ?>% T
                                        </span>
                                        <span class="inline-flex items-center rounded-md dark:bg-blue-700 bg-blue-50 px-2 py-1 text-xs font-medium dark:text-blue-50 text-blue-700 ring-1 ring-inset dark:ring-blue-700/10 ring-blue-700/10">
                                            <?php echo $row['metadata']['death']['ct']; ?>% CT
                                        </span>
                                    </div>
                                    <div class="flex gap-1">
                                        <span class="inline-flex items-center rounded-md dark:bg-gray-700 bg-gray-50 px-2 py-1 text-xs font-medium dark:text-gray-50 text-gray-600 ring-1 ring-inset dark:ring-gray-500/10 ring-gray-500/10">
                                            <?php echo $row['metadata']['death']['suicide']; ?>% Suicide
                                        </span>
                                    </div>
                                </div>
                            </td>
                            <td class="px-6 py-4">
                                <div class="flex gap-2">
                                    <span class="inline-flex items-center rounded-md dark:bg-gray-700 bg-gray-50 px-2 py-1 text-xs font-medium dark:text-gray-50 text-gray-600 ring-1 ring-inset dark:ring-gray-500/10 ring-gray-500/10">
                                        <?php echo $row['bomb_planted']; ?> planted
                                    </span>
                                    <span class="inline-flex items-center rounded-md dark:bg-gray-700 bg-gray-50 px-2 py-1 text-xs font-medium dark:text-gray-50 text-gray-600 ring-1 ring-inset dark:ring-gray-500/10 ring-gray-500/10">
                                        <?php echo $row['bomb_exploded']; ?> exploded
                                    </span>
                                    <span class="inline-flex items-center rounded-md dark:bg-gray-700 bg-gray-50 px-2 py-1 text-xs font-medium dark:text-gray-50 text-gray-600 ring-1 ring-inset dark:ring-gray-500/10 ring-gray-500/10">
                                        <?php echo $row['bomb_defused']; ?> defused
                                    </span>
                                </div>
                            </td>
                        </tr>
                    <?php } ?>
                </tbody>
            </table>
        </div>
    </div>
    
    <footer class="pt-5 flex gap-1 justify-center text-gray-400">
        <div class="flex gap-1 items-center">
            <a href="https://github.com/Salvatore-Als/cs2-rank" class="text-gray-400 hover:text-gray-600 transition">
                <i class="fab fa-github"></i>
            </a>
            <span>CS2 Rank by</span> 
            <a href="https://twitter.com/kriax_" class="text-blue-400 hover:text-blue-600 transition">Kriax</a>
        </div>
        <span>-</span>
        <div class="flex gap-1">
            <span>Sponsored by</span>
            <a href="https://www.verygames.net/en" class="text-blue-400 hover:text-blue-600 transition">VeryGames</a>
        </div>
    </footer>

    <script>
        const themeIcon = document.getElementById('theme-icon');
        const localStorageKey = 'cs2ranktheme';

        console.log("themeIcon", themeIcon);
        
        if(localStorage[localStorageKey] == 'dark') {
            document.documentElement.classList.add('dark');
        }

        switchIcon();

        function switchIcon() {
            themeIcon.classList = localStorage[localStorageKey] == 'dark' ? 'fa-solid fa-sun':'fa-solid fa-moon'
        }

        function changeTheme() {
            if(localStorage[localStorageKey] == 'dark') {
                localStorage.removeItem(localStorageKey);
                document.documentElement.classList.remove('dark');
            } else {
                localStorage[localStorageKey] = 'dark';
                document.documentElement.classList.add('dark');
            }

            switchIcon();
        }
    </script>
</body>
</html>