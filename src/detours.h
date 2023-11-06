#pragma once
#include "cdetour.h"
#include "entity/ccsplayercontroller.h"
#include "entity/ctakedamageinfo.h"

class CCheckTransmitInfo;
class IRecipientFilter;
class ISoundEmitterSystemBase;
class CBaseEntity;
class Z_CBaseEntity;
class CCSPlayerController;
class CEntityIndex;
class CCommand;
class CTriggerPush;
class CGameConfig;

bool InitDetours(CGameConfig *gameConfig);
void FlushAllDetours();