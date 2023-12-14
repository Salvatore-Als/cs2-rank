# CS2 Rank

**If you would like to support me in this project**

[![Texte alternatif](https://github.com/Salvatore-Als/cs2-rank/assets/58212852/809a130a-1c3a-4b37-aaf5-f72c88fc75e0)](https://www.paypal.com/donate/?hosted_button_id=UA272TAJVBT2U)

## b2.0.1 => b2.1.0
This version change change the database structure.
Before installing version `b2.1.0` if you are using an earlier beta version, you must import the sql file in https://github.com/Salvatore-Als/cs2-rank/tree/main/external/sql/migration-b201-b210.sql, it's not mandatory but without this proces you will lose all your rank.

Be sure to backup your Database before.

## Overview
CS2 Rank is an open source ranking plugin for Counter-Strike 2, relying on in-game events to award or deduct points. Player statistics are recorded in a MYSQL database.

###### **Developed in partnership with [VeryGames](https://www.verygames.net).**

## Feature
- Global Ranking System: Track your rank, top standings, and statistics globally, by map name, and specifically for the current map in play.
- Cross-Server Ranking: Enable cross-server ranking through rank referencing, allowing for a unified ranking experience across multiple servers.
- Free-for-All (FFA) Support: Enjoy comprehensive support for Free-for-All gameplay, ensuring a competitive experience for all players.
- Bot Compatibility: CS2 Rank seamlessly integrates with bots, enhancing the overall gaming environment.
- Translation and Colorization: CS2 Rank introduces a robust translation system and colorization of phrases, providing a personalized and visually appealing experience.
- Explore a Discord bot written in NodeJS, offering the ability to check specific player ranks, both personal and by map, as well as global and map-specific top rankings through rank referencing.
- Access a PHP-powered web interface providing top rankings by map and globally. This interface also utilizes rank referencing for seamless integration.

## Plugin
https://cs2rank.kriax.ovh/#/plugin
###### Required:
- CS2 Serveur (Linux or Window)
- Remove server hibernation `sv_hibernate_when_empty 0`
- mysql_mm plugins by Poggu : https://github.com/Poggicek/mysql_mm

###### My server is crashing during the boot:
The plugin is unloaded when there is a problem (MySQL, schema, etc.). To generate a crash report, you can utilize this plugin: https://github.com/komashchenko/AcceleratorLocal

###### Here are the steps to troubleshoot:

- Verify if mysql_mm is installed and functioning on your server by entering `meta list`.
- Ensure that your MySQL configuration in your plugin is accurate.

If the issue persists and results in a crash, please create an issue with the plugin list and include the server logs.

## Web page
https://cs2rank.kriax.ovh/#/web

https://github.com/Salvatore-Als/cs2-rank/tree/main/external/web

## Discord Bot
https://cs2rank.kriax.ovh/#/discord

https://github.com/Salvatore-Als/cs2-rank/tree/main/external/discord

## Discord API
https://cs2rank.kriax.ovh/#/public-api

https://github.com/Salvatore-Als/cs2-rank/tree/main/external/public-api

---
***Thank to CS2Fixe project (Poggy & Xen) for providing many feature.***
