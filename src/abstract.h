#pragma once

#include <ISmmPlugin.h>
#include <igameevents.h>
#include <iplayerinfo.h>
#include <sh_vector.h>
#include <stdint.h>
#include <playerslot.h>
#include "networksystem/inetworkserializer.h"
#include <iserver.h>

#define DEBUG_PREFIX "CS2 Rank - "
#define PREFIX " {RED}[CS2 Rank] "
#define PLUGIN_NAME "CS2 Rank"
#define PLUGIN_NAME_DEBUG "DEBUG - CS2 Rank"
#define PLUGIN_DESCRIPTION "Rank system"

#define GAMEDATA_PATH "addons/cs2rank/gamedata/cs2rank.games.txt"
#define POINTS_CONFIG_PATH "addons/cs2rank/configs/points.cfg"
#define PHRASES_CONFIG_PATH "addons/cs2rank/configs/phrases.cfg"
#define CORE_CONFIG_PATH "addons/cs2rank/configs/core.cfg"

#define MAX_QUERY_SIZES 8064

#define CS_TEAM_NONE 0
#define CS_TEAM_SPECTATOR 1
#define CS_TEAM_T 2
#define CS_TEAM_CT 3

#define MAXPLAYERS 64

#define HUD_PRINTNOTIFY 1
#define HUD_PRINTCONSOLE 2
#define HUD_PRINTTALK 3
#define HUD_PRINTCENTER 4

#ifdef _WIN32
#define ROOTBIN "/bin/win64/"
#define GAMEBIN "/csgo/bin/win64/"
#define MODULE_PREFIX ""
#define MODULE_EXT ".dll"
#else
#define ROOTBIN "/bin/linuxsteamrt64/"
#define GAMEBIN "/csgo/bin/linuxsteamrt64/"
#define MODULE_PREFIX "lib"
#define MODULE_EXT ".so"
#endif

enum RequestType
{
    Global = 0,
    Map = 1,
    Session = 2
};

void Debug(const char *, ...);
void Fatal(const char *, ...);
void Warn(const char *, ...);
size_t UTIL_Format(char *buffer, size_t maxlength, const char *fmt, ...);