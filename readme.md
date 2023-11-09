# Rank Plugin for Metamod with MySQL Integration

This project is not yet ready to be tested on a real server, it is still in development.
There's not a lot of work left before releasing a BETA :)

## Overview
**Developed in partnership with [VeryGames](https://www.verygames.net).**

This is a rank plugin designed to work with Metamod, incorporating a MySQL database for data storage. The plugin provides a point system that tracks player performance in a Counter-Strike 2. It includes various features to manage and update player ranks based on their in-game actions.

***Thank to CS2Fixe project (Poggy & Xen) for providing a solid code base.***

# Want to contribute ?
- If you want to contribute, you can check the todo list and make a pull request once you have made a feature.

- If a feature is not on the todo list, you can create an issue to discuss and check if we implement your idea :)

- If you don't have the required skills in C++, you can just test the plugin and create issues if needed :)

# TODO List

#### Security
- [ ]  Detect and unload plugin if hibernation is set as true
- [X]  Unload plugin if there is no mysql connection

#### Configuration
- [x]  Mysql
- [x]  Core
- [x]  Phrases
- [x]  Points

#### Configuration
- [x]  Translation system
- [x]  Colors

#### Commands
- [x]  !rank
- [x]  !restrank
- [x]  !top
- [ ]  !sessions (ranking from map)
- [ ]  !stats (HS percent, knife percent, death with percent T&CT, kill with percent T&CT)

#### Event triggers
- [x]  Player death by weapon
  - [x]  Attacker
  - [x]  Victim
  - [ ]  Assist
- [x]  Player death by headshot
  - [x]  Attacker
  - [x]  Victim
  - [ ]  Assist
- [x]  Player death by knife
  - [x]  Attacker
  - [x]  Victim
  - [ ]  Assist
- [ ]  Bomb planted
  - [x]  Team
  - [ ]  Planter (Need to fix the phrases that are not sent)
- [ ]  Bomb Exploded
  - [x]  Team
  - [ ]  Planter (Need to fix the phrases that are not sent)
  -  Need to fix the phrases that are not sent
- [ ]  Bomb Defused
  - [x]  Team
  - [ ]  Defuser (Need to fix the phrases that are not sent)

#### Stats triggers
- [ ]  Register last connection on connect and disconnect
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

#### Web page
- [ ]  Global rank system
- [ ]  Stats page per user

## Require
- CS2 Serveur (Linux or Window)
- Remove server hibernation `sv_hibernate_when_empty 0`
- mysql_mm plugins by Poggu : https://github.com/Poggicek/mysql_mm

## Features
Provider rank and stats system, as well a web interface

## Commands
The plugin includes the following commands:
- `!mm_rank` - Allows players to check their current rank and points.
- `!mm_top` - Displays the top-ranked players.
- `!mm_resetrank` - Resets a player's rank and points.
- `!mm_session` - Displays rank for the current map.
- `!mm!stats` - Displays players statistics.

## Configuration
The plugin offers a configuration file that allows you to customize its behavior according to your server's requirements. You can modify settings such as points gained or deducted for different actions and other gameplay-related parameters.

Moreover, support translation and colors !

- Points file: `addons/vgrank/configs/points.cfg`
- Translations file : `addons/vgrank/configs/phrases.cfg`
- Core file : `addons/vgrank/configs/core.cfg`