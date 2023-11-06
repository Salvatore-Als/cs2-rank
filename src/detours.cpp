#include "cdetour.h"
#include "module.h"
#include "addresses.h"
#include "detours.h"
#include "igameevents.h"
#include "gameconfig.h"
#include "abstract.h"
#include "entity/cbaseentity.h"
#include "entity/ctakedamageinfo.h"
#include "tier0/memdbgon.h"

extern CEntitySystem *g_pEntitySystem;
extern IGameEventManager2 *g_pGameEventManager;

CUtlVector<CDetourBase *> g_vecDetours;

bool InitDetours(CGameConfig *gameConfig)
{
	bool success = true;

	g_vecDetours.PurgeAndDeleteElements();

	return success;
}

void FlushAllDetours()
{
	g_vecDetours.Purge();
}