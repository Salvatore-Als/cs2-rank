# Rank page

## Overview
**Developed in partnership with [VeryGames](https://www.verygames.net).**

## Configuration
You can configure your webpage by editing `/src/config.php`

```
define("STEAMAPI_KEY", ""); // Your Steam WebAPI-Key found at https://steamcommunity.com/dev/apikey

define("MYSQL_HOST", "localhost"); // Mysql host
define("MYSQL_PASSWORD", "8io/PAz838st03"); // Mysql password
define("MYSQL_DATABASE", "cs2"); // Mysql database
define("MYSQL_USER", "root"); // Mysql user

define("PLAYERS_PER_PAGE", "50");   // Players display per page
define("MINIMUM_POINTS", 10); // Minimum points to be on the rank, should be the same config as your plugin connfig
```

## Preview 
https://prnt.sc/uNZIyTaTY8Zr

https://prnt.sc/6nzllbxfRFfb

## Theme
The website using **Tailwind**, if you want to make an update and compile a custom css you can use `npm run build:css:dev` and then `npm run build:css` to compile :) 
Don't forget to type `npm install` before!

## Running
You must upload the content of the `src` folder to your website, I advise you to make a specific site or folder.

## Api ?
There is no API, it's a simple web page, but you can use getRank.php to get the rank `https://yoursite.com.getRank.php?page=1&server=ffa`. For example on a discord bot :)
This page return this structure : 
```
    {
        "currentPage": 1,
        "totalPage": 12,
        "server": "ffa",
        "results: [
            {
                "authid": 76561198018935405,
                "name": "kriax2",
                "metadata": {
                    "rank": 1,
                    "kill": {
                        "total": 84,
                        "ct": 54,
                        "t": 45,
                        "headshot": 16,
                        "knife": 53
                    },
                    "death": {
                        "total": 59,
                        "ct": 35,
                        "t": 64,
                        "suicide": 67
                    },
                    "killassist": {
                        "total": 174,
                        "ct": 70,
                        "t": 29
                    }
                }
            },
            ... Other players
        ]
    }
```
