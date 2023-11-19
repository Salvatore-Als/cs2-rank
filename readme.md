# Rank Plugin for Metamod with MySQL Integration

### ALPHA TEST

I'm excited to share that our plugin is currently undergoing private alpha testing. However, to ensure a more focused and effective testing experience, I have not yet released a public alpha version.

Fear not, a public release will be made available as soon as alpha testing is concluded, and the plugin reaches a satisfactory level of stability. I appreciate your understanding and patience during this period.

If you're interested in participating in the private alpha testing and contributing to shaping the future of the plugin, feel free to reach out to me on via Discord `_kriax`. Your involvement is valuable and will help make this plugin an outstanding solution.

**If you use the plugin after compiling it yourself I would consider you an alpha tester :)**

## Overview
**Developed in partnership with [VeryGames](https://www.verygames.net).**

This is a rank plugin designed to work with Metamod, incorporating a MySQL database for data storage. The plugin provides a point system that tracks player performance in a Counter-Strike 2. It includes various features to manage and update player ranks based on their in-game actions.

***Thank to CS2Fixe project (Poggy & Xen) for providing a solid code base.***

# Want to contribute ?
- If you want to contribute, you can check the todo list and make a pull request once you have made a feature.

- If a feature is not on the todo list, you can create an issue to discuss and check if we implement your idea :)

- If you don't have the required skills in C++, you can just test the plugin and create issues if needed :)

# TODO List

#### Beta Fix
- [ ] Remove points lost when disconnecting or changing teams for Spectator
- [ ] Add a config `enable_ffa` to disable the points lost during a teamkill (it's better than used a game mode detection)
- [ ] Fix escape string for player's name
- [ ] Add a map reference for the rank with !maprank !mapsession !maptop
- [x] WEB: Fix date view
- [x] WEB: Fix CT kills percentage  

#### Security
- [X]  For hibernation to false
- [X]  Unload plugin if there is no mysql connection

#### Configuration
- [x]  Mysql
- [x]  Core
- [x]  Phrases
- [x]  Points

#### Core
- [x]  Rank stats
- [x]  Sessions stats
- [x]  Translation system
- [x]  Colors
- [x]  Cross server ranking
- [ ]  Deathmatch FFA support
- [ ]  Documentation (In progress)

#### Commands
- [x]  Antiflood system
- [x]  !rankh
- [x]  !rankannouce
- [x]  !rank
- [x]  !stats
- [x]  !top
- [x]  !restrank
- [x]  !topsession
- [x]  !ranksession
- [x]  !statsession

#### Event triggers
- [x]  Player death by weapon
  - [x]  Attacker
  - [x]  Victim
  - [X]  Assist
- [x]  Player death by headshot
  - [x]  Attacker
  - [x]  Victim
  - [X]  Assist
- [x]  Player death by knife
  - [x]  Attacker
  - [x]  Victim
  - [X]  Assist
- [x]  Bomb planted
  - [x]  Team
  - [x]  Planter
- [x]  Bomb Exploded
  - [x]  Team
  - [X]  Planter
- [x]  Bomb Defused
  - [x]  Team
  - [x]  Defuser

#### Stats triggers
- [x]  Register last connection
- [ ]  Stats per weapons
- [x]  Headshot
- [x]  Knife
- [x]  Kill T
- [x]  Kill CT
- [x]  Death T
- [x]  Death CT
- [x]  Death Suicide
- [x]  Bomb planted
- [x]  Bomb exploded
- [x]  Bomb defused
- [x]  Teamkill T
- [x]  Teamkill CT
- [ ] All others events

#### Web page
- [x]  Theme
- [x]  Lazy load images
- [ ]  Global rank system
  - [x]  Table with rank
  - [ ]  Player searching
  - [x]  Pagination
- [ ]  Stats page per user (After the end of statistics)

#### Discord Bot
- [x]  Translation
- [x]  Command `/rank`
- [x]  Command `/top`
- [x]  Command `/ranklink`

## Require
- CS2 Serveur (Linux or Window)
- Remove server hibernation `sv_hibernate_when_empty 0`
- mysql_mm plugins by Poggu : https://github.com/Poggicek/mysql_mm

## Features
Provide rank and stats system.

A web interface is available https://github.com/Salvatore-Als/cs2-rank/tree/main/external/web

A discord bot is available https://github.com/Salvatore-Als/cs2-rank/tree/main/external/discord

## Commands
The plugin includes the following commands:
- `!rankh` - Display available commands.
- `!rankannouce` - Allows players to disable/enable points annoucement.
- `!rank` - Allows players to check their current rank and points.
- `!stats` - Displays players statistics.
- `!top` - Display the top-ranked players.
- `!resetrank` - Resets a player's rank and points.
- `!ranksession` - Displays rank for the current map.
- `!topsessions` - Displays players statistics for the current map;
- `!statssessions` - Displays players statistics from for current map.

## Configuration
The plugin offers a configuration file that allows you to customize its behavior according to your server's requirements. You can modify settings such as points gained or deducted for different actions and other gameplay-related parameters.

Moreover, support translation and colors !

- Points file: `addons/vgrank/configs/points.cfg`
- Translations file : `addons/vgrank/configs/phrases.cfg`
- Core file : `addons/vgrank/configs/core.cfg`
