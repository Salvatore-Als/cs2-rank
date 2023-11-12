<?php 
    include 'config.php';

    $default = "https://avatars.cloudflare.steamstatic.com/fef49e7fa7e1997310d705b2a6158ff8dc1cdfeb_full.jpg";

    if (empty($_GET['authid']) || strlen($_GET['authid']) != 17) {
        echo $default;
        exit(404);
    }

    $authid = htmlspecialchars($_GET['authid']);

    $url = file_get_contents("https://api.steampowered.com/ISteamUser/GetPlayerSummaries/v0002/?key=".STEAMAPI_KEY."&steamids=".$authid); 
    $content = json_decode($url, true);

    $avatar = $content['response']['players'][0]['avatarmedium'];
    echo is_null($avatar) || $avatar == "" ? $defaut : $avatar;
?>