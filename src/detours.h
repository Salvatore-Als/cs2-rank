#pragma once
#include "abstract.h"
#include "cdetour.h"
#include "entity/ccsplayercontroller.h"

class CCSPlayerController;
class CCommand;
class CGameConfig;

bool InitDetours(CGameConfig *gameConfig);
void FlushAllDetours();

void FASTCALL Detour_Host_Say(CCSPlayerController *, CCommand &, bool, int, const char *);
extern CDetour<decltype(Detour_Host_Say)> Host_Say;