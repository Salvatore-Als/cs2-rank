#include "abstract.h"
#include "KeyValues.h"
#include "eventlistener.h"
#include "entity/ccsplayercontroller.h"
#include "chat.h"
#include "tier0/memdbgon.h"
#include "player.h"
#include "ctimer.h"
#include "config.h"

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

    g_CChat->PrintToChat(pPlanterController, "+%i points for planted the bomb", g_CConfig->GetPointsWinBombPlantedPlayer());
    pPlayer->SetPoints(pPlayer->GetPoints() + g_CConfig->GetPointsWinBombPlantedPlayer());

    g_CPlayerManager->AddTeamPoint(CS_TEAM_T, g_CConfig->GetPointsWinBombPlantedTeam());
    g_CChat->PrintToChatT("+%i point all T for planted the bomb", g_CConfig->GetPointsWinBombPlantedTeam());
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

    g_CChat->PrintToChat(pDefuserController, "+%i points for defused the bomb", g_CConfig->GetPointsWinBombDefusedPlayer());
    pPlayer->SetPoints(pPlayer->GetPoints() + g_CConfig->GetPointsWinBombDefusedPlayer());

    g_CChat->PrintToChatT("+%i point all CT for defused the bomb", g_CConfig->GetPointsWinBombDefusedTeam());
    g_CPlayerManager->AddTeamPoint(CS_TEAM_CT, g_CConfig->GetPointsWinBombDefusedTeam());
}

GAME_EVENT_F(bomb_exploded)
{
    Debug("EVENT: bomb_exploded");

    // TODO: exploded player
    g_CChat->PrintToChatT("+%i point all T for exploded the bomb", g_CConfig->GetPointsWinBombExplodedTeam());
    g_CPlayerManager->AddTeamPoint(CS_TEAM_T, g_CConfig->GetPointsWinBombExplodedTeam());
}

GAME_EVENT_F(player_spawn)
{
    CCSPlayerController *pController = (CCSPlayerController *)pEvent->GetPlayerController("userid");

    if (!pController)
    {
        return;
    }

    CHandle<CCSPlayerController> hController = pController->GetHandle();

    // Gotta do this on the next frame...
    new CTimer(0.0f, false, [hController]()
               {
        CCSPlayerController *pController = hController.Get();

        if(!pController)
            return -1.0f;

        CBasePlayerPawn *pPawn = pController->m_hPawn();

        if(!pPawn)
            return -1.0f;
        
        pPawn->m_MoveType = MOVETYPE_NONE;

		return -1.0f; });
}

GAME_EVENT_F(player_death)
{
    Debug("EVENT: player_death");

    CCSPlayerController *pVictimController = (CCSPlayerController *)pEvent->GetPlayerController("userid");
    CCSPlayerController *pAttackerController = (CCSPlayerController *)pEvent->GetPlayerController("attacker");

    if (!pVictimController || !pAttackerController)
    {
        Debug("- Invalid pVictimController (%p) or pAttackerController (%p)", pVictimController, pAttackerController);
        return;
    }

    CRankPlayer *pVictim = pVictimController->GetRankPlayer();
    CRankPlayer *pAttacker = pAttackerController->GetRankPlayer();

    // Disable if invalid player
    if (!pVictim || !pAttacker || !pAttacker->IsValidPlayer())
    {
        Debug("- Invalid pVictim (%p bot : %i) or pAttacker (%p bot %i)", pVictim, pVictim->IsFakeClient(), pAttacker, pAttacker->IsFakeClient());
        return;
    }

    if (pVictim->IsFakeClient())
    {
        Debug("- Victim is a fake client");
        // TODO: RETURN
    }

    // Suicide
    if (pVictimController == pAttackerController)
    {
        // OK
        g_CChat->PrintToChat(pVictimController, "-%i points for suicide", g_CConfig->GetPointsLooseSuicide());
        pVictim->SetPoints(pVictim->GetPoints() - g_CConfig->GetPointsLooseSuicide());

        return;
    }

    // Teamkill, TODO: try with a deathmatch type FFA, not taking account if it's enable
    // OK
    if (pVictimController->m_iTeamNum.Get() == pAttackerController->m_iTeamNum.Get())
    {
        g_CChat->PrintToChat(pAttackerController, "-%i points for team kill", g_CConfig->GetPointsLooseTeamkill());
        pAttacker->SetPoints(pAttacker->GetPoints() - g_CConfig->GetPointsLooseTeamkill());

        return;
    }

    const char *weapon = pEvent->GetString("weapon");

    Debug("- With weapon %s", weapon);

    // OK
    if (strstr(weapon, "knife") != nullptr)
    {
        pAttacker->SetPoints(pAttacker->GetPoints() + g_CConfig->GetPointsWinKillKnife());
        g_CChat->PrintToChat(pAttackerController, "+%i points for killing with knife", g_CConfig->GetPointsWinKillKnife());

        if (!pVictim->IsFakeClient())
        {
            pVictim->SetPoints(pVictim->GetPoints() - g_CConfig->GetPointsLooseKillKnife());
            g_CChat->PrintToChat(pVictimController, "-%i points for killing with knife", g_CConfig->GetPointsLooseKillKnife());
        }

        return;
    }

    bool bHeadshot = pEvent->GetBool("headshot");

    // TODO: add assist

    if (bHeadshot)
    {
        // OK
        g_CChat->PrintToChat(pAttackerController, "+%i point(s) for killing with headshot", g_CConfig->GetPointsWinKillWeaponHs());
        pAttacker->SetPoints(pAttacker->GetPoints() + g_CConfig->GetPointsWinKillWeaponHs());

        if (!pVictim->IsFakeClient())
        {
            pVictim->SetPoints(pVictim->GetPoints() - g_CConfig->GetPointsLooseKillWeaponHs());
            g_CChat->PrintToChat(pVictimController, "-%i points for dying with headshot", g_CConfig->GetPointsLooseKillWeaponHs());
        }
    }
    else
    {
        // OK
        g_CChat->PrintToChat(pAttackerController, "+%i point(s) for killing", g_CConfig->GetPointsWinKillWeapon());
        pAttacker->SetPoints(pAttacker->GetPoints() + g_CConfig->GetPointsWinKillWeapon());

        if (!pVictim->IsFakeClient())
        {
            pVictim->SetPoints(pVictim->GetPoints() - g_CConfig->GetPointsLooseKillWeapon());
            g_CChat->PrintToChat(pVictimController, "-%i points for dying", g_CConfig->GetPointsLooseKillWeapon());
        }
    }
}