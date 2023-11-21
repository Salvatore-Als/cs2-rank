# Rank Plugin for Metamod with MySQL Integration

# PRIVATE ALPHA
This plugin is on a **private alpha** test, a public release will be made after this test :)

Documentation website is available here https://cs2rank.kriax.ovh

**If you would like to support me in this project**

[![Texte alternatif](https://github.com/Salvatore-Als/cs2-rank/assets/58212852/809a130a-1c3a-4b37-aaf5-f72c88fc75e0)](https://www.paypal.com/donate/?hosted_button_id=UA272TAJVBT2U)

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
- [x] DISCORD : Manage rank per map
- [x] DISCORD : Manage top per map
- [x] DISCORD : Add /rankmaps to display the available map

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
- !statssession

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
