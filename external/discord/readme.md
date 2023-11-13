# Rank bot

## Overview
**Developed in partnership with [VeryGames](https://www.verygames.net).**

## Configuration
You need to configure your bot by using a `.env` file.

```
DISCORD_TOKEN = "" // Your discord token
DISCORD_CLINETID = "" // Your bot client id

LOCALE = "fr" // Local to used for date format and translation

MYSQL_HOST = "localhost"
MYSQL_PASSWORD = "root"
MYSQL_DATABASE = "cs2"
MYSQL_USER = "root"
MYSQL_PORT = 3306
```

## Preview

## Translations
The bot provide a simple translate system. You can create your own translate file.
The translate file should be a local `fr.json` `en.jon` `gb.json` `etc.json`. If a translate is not found, thre is a fallback on the default translation file, so don't delete it.

## Running
First type `npm install` and then `npm start`