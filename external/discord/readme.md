# Rank Bot

## Overview
**Developed in partnership with [VeryGames](https://www.verygames.net).**

## Features
### 1. `/rank <group> <player/steamid64> <map>`
- This command displays your rank. The group is the rank reference (from the server).

### 2. `/top <group>  <map>`
- This command displays the top. The group is the rank reference (from the server).

### 3. `/ranklink <steamid64>`
- This command links your Discord account to your Steam account. With a linked account, `/rank <group>` works without a search string.

## How Linked Accounts Work
After typing `/ranklink <steamid64>`, the bot gives you a unique name. You need to change your Steam account name and retype `/ranklink <steamid64>`. 
If the name matches, you will be registered in the database.

The link name is available for 5 minutes; after this period, you will need to repeat the process.

## How to Run the Bot?
The bot is written with Nodejs and Typescript.
- 1. Run `npm install`.
- 2. Create a .env file at the root path:

```env
DISCORD_TOKEN = "" // Your Discord token

LOCALE = "default" // Locale used for date format and translation (fr / en / etc)

MINIMUM_POINTS = 10 // Minimum points to be on the rank

MYSQL_HOST = "localhost"
MYSQL_PASSWORD = "root"
MYSQL_DATABASE = "cs2"
MYSQL_USER = "root"
MYSQL_PORT = 3306
```

- 3. Run `npm start`

## Translations
The bot provides a simple translation system. You can create your own translation file. The translation file should be named `fr.json`, `en.json`, `gb.json`, etc. 
If a translation is not found, there is a fallback to the default translation file, so don't delete it.

The locale can be configured in the `.env` file. 

There is a variable interpolation, by using `#VAR#` as example 
`Hello Kriax, you have 20 points !` will be translated by `Hello #VAR#, you have #VAR# points`.

Discord use `markdown` format, you can use this format for your translation.

***Keep in mine that the var order is important, this one can not change !***

## Preview
`/rank` 
  - https://prnt.sc/7jOshuO9EqPG

`/top` 
  - https://prnt.sc/WwDGGYa0JtOr

`/ranklink` 
  - https://prnt.sc/GoqzbDPozR8d 
  - https://prnt.sc/Ed6QTB00xCOO 
  - https://prnt.sc/pcoJ007wfq0F

More screenshot
  - https://prnt.sc/RxnXb_a-Nud7
