
#pragma once

#include "cbaseentity.h"
#include "cbaseplayercontroller.h"
#include "cbasemodelentity.h"

class CBaseCombatWeapon;

class CBasePlayerPawn : public CBaseModelEntity
{
public:
    DECLARE_SCHEMA_CLASS(CBasePlayerPawn);
    SCHEMA_FIELD(CHandle<CBasePlayerController>, m_hController)
};