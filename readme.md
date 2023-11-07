# Rank Plugin for Metamod with MySQL Integration

## Overview
**Developed in partnership with VeryGames.**

This is a rank plugin designed to work with Metamod, incorporating a MySQL database for data storage. The plugin provides a point system that tracks player performance in a Counter-Strike 2. It includes various features to manage and update player ranks based on their in-game actions.

***Thank to CS2Fixe project (Poggy & Xen) for providing a solid code base.***

## Features
- Killing a player results in points being added to the attacker and deducted from the victim's This system takes headshots and knife kills into account.
- When a player commits suicide, it results in a points deduction.
- Killing a teammate also leads to a points deduction.
- Planting the bomb awards points to the planter and to the terrorists.
- If the bomb explodes, additional points are awarded to the terrorists.
- Defusing the bomb awards points to the defuser and to the counter-terrorists.

## Commands
The plugin includes the following commands:
- `!mm_rank` - Allows players to check their current rank and points.
- `!mm_top` - Displays the top-ranked players.
- `!mm_resetrank` - Resets a player's rank and points.
- `!mm_session` - Displays session statistics.

## Configuration
The plugin offers a configuration file that allows you to customize its behavior according to your server's requirements. You can modify settings such as points gained or deducted for different actions and other gameplay-related parameters.

TODO: config file

## Web Page
Additionally, the plugin comes with a web page feature that enables you to display player statistics. This web page can be accessed through a web browser and provides an easy way for players to view their in-game progress and ranking.

TODO: config file