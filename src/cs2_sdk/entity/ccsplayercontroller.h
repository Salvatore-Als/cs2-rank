#pragma once

#include "cbaseplayercontroller.h"
#include "gameconfig.h"
#include "player.h"

extern CEntitySystem *g_pEntitySystem;

class CCSPlayerController : public CBasePlayerController
{
public:
    DECLARE_SCHEMA_CLASS(CCSPlayerController);

    SCHEMA_FIELD(bool, m_bPawnIsAlive);
    
    static CCSPlayerController *FromSlot(CPlayerSlot slot)
    {
        return (CCSPlayerController *)g_pEntitySystem->GetBaseEntity(CEntityIndex(slot.Get() + 1));
    }

    CRankPlayer* GetRankPlayer()
	{
		return g_CPlayerManager->GetPlayer(GetPlayerSlot());
	}

    bool IsController()
    {
        static int offset = g_CGameConfig->GetOffset("IsEntityController");
        return CALL_VIRTUAL(bool, offset, this);
    }
};