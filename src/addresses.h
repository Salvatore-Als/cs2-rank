#pragma once

#include "abstract.h"
#include "utils/module.h"
#include "gameconfig.h"

//extern CGameConfig *g_CGameConfig;

class CBasePlayerController;
class CCSPlayerController;
class CEntityInstance;
class CGameConfig;
class CBaseEntity;
class CCSPlayerPawn;

class CAddresses
{
public:
    CAddresses()
    {

    }

    void Init(CGameConfig *g_CGameConfig);  // using it b'cause of cirtular dependencies

    CModule *GetModuleServer() { return g_moduleServer; }
    CModule *GetModuleEngine() { return g_moduleEngine; }
    CModule *GetShemaSystem() { return g_schemasystem; }

    void (*NetworkStateChanged)(int64 chainEntity, int64 offset, int64 a3);
    void (*StateChanged)(void *networkTransmitComponent, CEntityInstance *ent, int64 offset, int16 a4, int16 a5);

    void (*ClientPrint)(CBasePlayerController *player, int msg_dest, const char *msg_name, const char *param1, const char *param2, const char *param3, const char *param4);
    void (*UTIL_ClientPrintAll)(int msg_dest, const char *msg_name, const char *param1, const char *param2, const char *param3, const char *param4);

    void Clean();

private:
    CModule *g_moduleServer;
    CModule *g_schemasystem;
    CModule *g_moduleEngine;
};

extern CAddresses *g_CAddresses;