#pragma once
#include "abstract.h"
#include "cdetour.h"
#include "entity/ccsplayercontroller.h"

class CCSPlayerController;
class CCommand;
class CGameConfig;

bool InitDetours(CGameConfig *gameConfig);
void FlushAllDetours();