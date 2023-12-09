# CS2 Rank - Public API

## Overview
**Developed in partnership with [VeryGames](https://www.verygames.net).**

## How to Run the Bot?
The bot is written with Nodejs and Typescript.
- 1. Run `npm install`.
- 2. Create a .env file at the root path:

```env
PORT = 3000

MINIMUM_POINTS = 10 // Minimum points to be on the rank

MYSQL_HOST = "localhost"
MYSQL_PASSWORD = "root"
MYSQL_DATABASE = "cs2"
MYSQL_USER = "root"
MYSQL_PORT = 3306
```

- 3. Run `npm start`

## Routes
* : Mandatory query option

### `/players/player/${steamid64}`
### `/players/player/${name}`
Returns the statistics of a player
- `group: string` *Group to fetch the player 
- `map: string` Map to fetch the player, not used for global statistics
```
{
    ...player
}
```

### `/players/top`
Returns the top of a player
- `group: string` *Group to fetch the top 
- `map: string` Map to fetch the top, not used for global top
```
[
    {
        ...player
    }
]
```

### `/groups`
Returns the list of groups
```
[
    {
        ...group
    }
]
```

### `/maps`
Returns the list of maps
```
[
    {
        ...map
    }
]
```

## Structures

### `player`
```{
    "user_id": number,
    "points": number,
    "lastconnect": number,
    "death_suicide": number,
    "death_t": number,
    "death_ct": number,
    "bomb_planted": number,
    "bomb_exploded": number,
    "bomb_defused": number,
    "kill_knife": number,
    "kill_headshot": number,
    "kill_t": number,
    "kill_ct": number,
    "teamkill_ct": number,
    "teamkill_t": number,
    "killassist_t": number,
    "killassist_ct": number,
    "authid": number,
    "name": string,
    "ignore_annouce": boolean,
    "rank": number
}```

### `map`
```{
    "id": number,
    "name": string,
}```

### `group`
```{
    "id": number,
    "name": string,
    "custom_name": string
}```