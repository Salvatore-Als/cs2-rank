#pragma once

#include "abstract.h"
#include <playerslot.h>
#include "utils/module.h"
#include "eiface.h"
#include "interfaces/cgameresourceserviceserver.h"
#include "entity2/entitysystem.h"
#include "addresses.h"

class CBasePlayerController;

extern CEntitySystem *g_pEntitySystem;

class CChat
{
public:
    CChat()
    {
  
    }

    void PrintToChatAll(const char *msg, ...);
    void PrintToChatCT(const char *msg, ...);
    void PrintToChatT(const char *msg, ...);

    void PrintToChat(CPlayerSlot slot, const char *msg, ...);
    void PrintToChat(CBasePlayerController *player, const char *msg, ...);

private:
    void PrintToChatTeam(int teamIndex, const char *msg);
};

extern CChat *g_CChat;