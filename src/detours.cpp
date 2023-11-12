#include "cdetour.h"
#include "module.h"
#include "addresses.h"
#include "detours.h"
#include "igameevents.h"
#include "gameconfig.h"
#include "abstract.h"
#include "entity/ccsplayercontroller.h"
#include "tier0/memdbgon.h"
#include "basecommands.h"
#include "chat.h"
#include "config.h"

extern CEntitySystem *g_pEntitySystem;
extern IGameEventManager2 *g_pGameEventManager;

CUtlVector<CDetourBase *> g_vecDetours;

DECLARE_DETOUR(Host_Say, Detour_Host_Say);

void FASTCALL Detour_Host_Say(CCSPlayerController *pController, CCommand &args, bool teamonly, int unk1, const char *unk2)
{
	bool bFlooding = pController && pController->GetRankPlayer()->IsFlooding();

	if (*args[1] != '/' && !bFlooding)
	{
		Host_Say(pController, args, teamonly, unk1, unk2);

		if (pController)
		{
			IGameEvent *pEvent = g_pGameEventManager->CreateEvent("player_chat");

			if (pEvent)
			{
				pEvent->SetBool("teamonly", teamonly);
				pEvent->SetInt("userid", pController->entindex());
				pEvent->SetString("text", args[1]);

				g_pGameEventManager->FireEvent(pEvent, true);
			}
		}
	}

	if (*args[1] == '!' || *args[1] == '/')
	{
		if (bFlooding)
			g_CChat->PrintToChat(pController, false, g_CConfig->Translate("CHAT_FLOODING"));
		else
			ParseChatCommand(args.ArgS() + 1, pController);
	}
}

bool InitDetours(CGameConfig *gameConfig)
{
	bool success = true;

	g_vecDetours.PurgeAndDeleteElements();

	if (!Host_Say.CreateDetour(gameConfig))
		success = false;

	Host_Say.EnableDetour();

	return success;
}

void FlushAllDetours()
{
	g_vecDetours.Purge();
}