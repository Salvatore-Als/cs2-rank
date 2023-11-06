#include "abstract.h"
#include "KeyValues.h"
#include "eventlistener.h"
#include "entity/ccsplayercontroller.h"
#include "chat.h"
#include "tier0/memdbgon.h"
#include "player.h"

extern IGameEventManager2 *g_pGameEventManager;
extern IServerGameClients *g_pSource2GameClients;
extern CEntitySystem *g_pEntitySystem;
// extern CChat *g_CChat;

CUtlVector<CGameEventListener *> g_vecEventListeners;

void RegisterEventListeners()
{
    if (!g_pGameEventManager)
    {
        Fatal("Unable to RegisterEventListeners, g_pGameEventManager is null");
        return;
    }

    FOR_EACH_VEC(g_vecEventListeners, i)
    {
        g_pGameEventManager->AddListener(g_vecEventListeners[i], g_vecEventListeners[i]->GetEventName(), true);
    }
}

void UnregisterEventListeners()
{
    if (!g_pGameEventManager)
    {
        return;
    }

    FOR_EACH_VEC(g_vecEventListeners, i)
    {
        g_pGameEventManager->RemoveListener(g_vecEventListeners[i]);
    }

    g_vecEventListeners.Purge();
}

GAME_EVENT_F(round_end)
{
    // Update all player
    for (int i = 0; i < g_pGlobals->maxClients; i++)
    {
        CCSPlayerController *pController = CCSPlayerController::FromSlot(i);

        if (!pController)
        {
            continue;
        }

        CRankPlayer *pPlayer = pController->GetRankPlayer();
        if (!pPlayer || !pPlayer->IsValidPlayer())
        {
            continue;
        }

        pPlayer->SaveOnDatabase();
    }
}

GAME_EVENT_F(bomb_planted)
{
    // TODO: find why userid is not valid

    Debug("EVENT: bomb_planted");
    CCSPlayerController *pPlanterController = (CCSPlayerController *)pEvent->GetPlayerController("userid");
    ;

    if (!pPlanterController)
    {
        Debug("- Invalid pPlanterController");
        return;
    }

    CRankPlayer *pPlayer = pPlanterController->GetRankPlayer();

    if (!pPlayer || !pPlayer->IsValidPlayer())
    {
        Debug("- Invalid pPlayer");
        return;
    }

    g_CChat->PrintToChat(pPlanterController, "+2 points for planted the bomb");
    pPlayer->SetPoints(pPlayer->GetPoints() + 2);

    g_CPlayerManager->AddTeamPoint(CS_TEAM_T, 1);
    g_CChat->PrintToChatT("+1 point all T for planted the bomb");
}

GAME_EVENT_F(bomb_defused)
{
    // TODO: find why userid is not valid

    Debug("EVENT: bomb_defused");

    CCSPlayerController *pDefuserController = (CCSPlayerController *)pEvent->GetPlayerController("userid");

    if (!pDefuserController)
    {
        Debug("- Invalid pDefuserController");
        return;
    }

    CRankPlayer *pPlayer = pDefuserController->GetRankPlayer();

    if (!pPlayer || !pPlayer->IsValidPlayer())
    {
        Debug("- Invalid pPlayer");
        return;
    }

    g_CChat->PrintToChat(pDefuserController, "+2 points for defused the bomb");
    pPlayer->SetPoints(pPlayer->GetPoints() + 2);

    g_CChat->PrintToChatT("+1 point all CT for defused the bomb");
    g_CPlayerManager->AddTeamPoint(CS_TEAM_CT, 1);
}

GAME_EVENT_F(bomb_exploded)
{
    Debug("EVENT: bomb_exploded");

    // OK
    g_CChat->PrintToChatT("+1 point all T for exploded the bomb");
    g_CPlayerManager->AddTeamPoint(CS_TEAM_T, 1);
}

GAME_EVENT_F(player_death)
{
    Debug("EVENT: player_death");

    CCSPlayerController *pVictimController = (CCSPlayerController *)pEvent->GetPlayerController("userid");
    CCSPlayerController *pAttackerController = (CCSPlayerController *)pEvent->GetPlayerController("attacker");

    if (!pVictimController || !pAttackerController)
    {
        return;
    }

    CRankPlayer *pVictim = pVictimController->GetRankPlayer();
    CRankPlayer *pAttacker = pAttackerController->GetRankPlayer();

    // Disable if invalid player
    if (!pVictim || !pVictim->IsValidPlayer() || !pAttacker || !pAttacker->IsValidPlayer())
    {
        return;
    }

    // Suicide
    if (pVictimController == pAttackerController)
    {
        // OK
        g_CChat->PrintToChat(pVictimController, "-2 points for suicide");
        pVictim->SetPoints(pVictim->GetPoints() - 2);
        // -2
        return;
    }

    Debug("- pVictimController team %i", pVictimController->m_iTeamNum.Get());
    Debug("- pAttackerController team %i", pAttackerController->m_iTeamNum.Get());

    // Teamkill
    // OK
    if (pVictimController->m_iTeamNum.Get() == pAttackerController->m_iTeamNum.Get())
    {
        g_CChat->PrintToChat(pAttackerController, "-3 points for team kill");
        pAttacker->SetPoints(pAttacker->GetPoints() - 2);

        return;
    }

    const char *weapon = pEvent->GetString("weapon");

    Debug("- With weapon %s", weapon);

    // OK
    if (strstr(weapon, "knife") != nullptr)
    {
        g_CChat->PrintToChat(pAttackerController, "+3 points for killing with knife");
        g_CChat->PrintToChat(pVictimController, "-3 points for team kill");

        pAttacker->SetPoints(pAttacker->GetPoints() + 3);
        pVictim->SetPoints(pVictim->GetPoints() - 3);

        return;
    }

    bool bHeadshot = pEvent->GetBool("headshot");

    if (bHeadshot)
    {
        // OK
        g_CChat->PrintToChat(pAttackerController, "+4 point(s) for killing with headshot");
        g_CChat->PrintToChat(pVictimController, "-2 points for dying with headshot");

        pAttacker->SetPoints(pAttacker->GetPoints() + 4);
        pVictim->SetPoints(pVictim->GetPoints() - 2);
    }
    else
    {
        // OK
        g_CChat->PrintToChat(pAttackerController, "+2 point(s) for killing");
        g_CChat->PrintToChat(pVictimController, "-2 points for dying");

        pAttacker->SetPoints(pAttacker->GetPoints() + 2);
        pVictim->SetPoints(pVictim->GetPoints() - 2);
    }
}