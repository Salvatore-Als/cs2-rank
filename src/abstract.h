#pragma once

#include <ISmmPlugin.h>
#include <igameevents.h>
#include <iplayerinfo.h>
#include <sh_vector.h>
#include <stdint.h>
#include <playerslot.h>
#include "networksystem/inetworkserializer.h"
#include <iserver.h>

#define DEBUG_PREFIX "VG Rank - "
#define PREFIX " {RED}[VG Rank] "
#define PLUGIN_NAME "VeryGames Rank"
#define PLUGIN_DESCRIPTION "Rank system"

#define GAMEDATA_PATH "addons/vgrank/gamedata/vgrank.games.txt"
#define POINTS_CONFIG_PATH "addons/vgrank/configs/points.cfg"
#define PHRASES_CONFIG_PATH "addons/vgrank/configs/phrases.cfg"
#define CORE_CONFIG_PATH "addons/vgrank/configs/core.cfg"

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

void Debug(const char *, ...);
void Fatal(const char *, ...);
void Warn(const char *, ...);
size_t UTIL_Format(char *buffer, size_t maxlength, const char *fmt, ...);