# Rank Plugin for Metamod with MySQL Integration

Documentation website is available here https://cs2rank.kriax.ovh

If you would like to support me in my projects : 

## Overview
**Developed in partnership with [VeryGames](https://www.verygames.net).**

This is a rank plugin designed to work with Metamod, incorporating a MySQL database for data storage. The plugin provides a point system that tracks player performance in a Counter-Strike 2. It includes various features to manage and update player ranks based on their in-game actions.

***Thank to CS2Fixe project (Poggy & Xen) for providing a solid code base.***

# Want to contribute ?
- Just make a PR :)

#### Beta Fix
- [x] Refactor player class
- [x] Remove points lost when disconnecting or changing teams for Spectator
- [x] Add a config `enable_ffa` to disable the points lost during a teamkill (it's better than used a game mode detection)
- [ ] Fix escape string for player's name ?
- [x] Add a map reference
- [x] Add rank per map
- [x] Add top per map
- [x] Add sessions per map
- [x] Add reset rank per map
- [x] WEB: Fix date view
- [x] WEB: Fix CT kills percentage  
- [ ] WEB : Manage top per map
- [ ] DISCORD : Manage rank per map
- [ ] DISCORD : Manage top per map
- [ ] DISCORD : Add /rankmaps to display the available map

#### Feature
- Rank
- Session
- Top
- Cross server ranking
- Map ranking
- FFA Support
- Translation
- Discord bot
- Web interface
- Documentation website

#### Commands
- !rankh
- !rankannouce
- !rank
- !rankmap
- !ranksession
- !top
- !topmap
- !topsession
- !resetrank
- !resetmaprank
- !stats
- !statsmap
- !statssessions

#### Web page
- Dark / Light theme
- Cross server ranking
- Map ranking
https://github.com/Salvatore-Als/cs2-rank/tree/main/external/web

#### Discord Bot
- Translation
- Command /rank
- Command /top
- Command /ranklink
- Command /rankmaps
https://github.com/Salvatore-Als/cs2-rank/tree/main/external/discord

## Require
- CS2 Serveur (Linux or Window)
- Remove server hibernation `sv_hibernate_when_empty 0`
- mysql_mm plugins by Poggu : https://github.com/Poggicek/mysql_mm
