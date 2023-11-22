#include "abstract.h"
#include "KeyValues.h"
#include "eventlistener.h"
#include "entity/ccsplayercontroller.h"
#include "chat.h"
#include "tier0/memdbgon.h"
#include "player.h"
#include "ctimer.h"
#include "config.h"
#include "main.h"

extern IGameEventManager2 *g_pGameEventManager;
extern IServerGameClients *g_pSource2GameClients;
extern CEntitySystem *g_pEntitySystem;

CUtlVector<CGameEventListener *> g_vecEventListeners;

void RegisterEventListeners()
{
    if (!g_pGameEventManager)
    {
        Fatal("Unable to RegisterEventListeners, g_pGameEventManager is null");
        g_CPlugin.ForceUnload();
        return;
    }

    FOR_EACH_VEC(g_vecEventListeners, i)
    g_pGameEventManager->AddListener(g_vecEventListeners[i], g_vecEventListeners[i]->GetEventName(), true);
}

void UnregisterEventListeners()
{
    if (!g_pGameEventManager)
        return;

    FOR_EACH_VEC(g_vecEventListeners, i)
    g_pGameEventManager->RemoveListener(g_vecEventListeners[i]);

    g_vecEventListeners.Purge();
}

GAME_EVENT_F(round_end)
{
    for (int i = 0; i < g_pGlobals->maxClients; i++)
    {
        CCSPlayerController *pController = CCSPlayerController::FromSlot(i);

        if (!pController)
            continue;

        CRankPlayer *pPlayer = pController->GetRankPlayer();
        if (!pPlayer || !pPlayer->IsValidPlayer())
            continue;

        pPlayer->SaveOnDatabase();
    }
}

GAME_EVENT_F(bomb_planted)
{
    if (!g_CConfig->IsMinimumPlayerReached())
        return;

    g_CPlayerManager->AddTeamPoint(CS_TEAM_T, g_CConfig->GetPointsWinBombPlantedTeam());

    char szTranslate[256];
    UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("BOMB_PLANT_TEAM"), g_CConfig->GetPointsWinBombPlantedTeam());
    g_CChat->PrintToChatAll(szTranslate);

    // Planter process

    CCSPlayerController *pPlanterController = (CCSPlayerController *)pEvent->GetPlayerController("userid");

    if (!pPlanterController)
        return;

    CRankPlayer *pPlanter = pPlanterController->GetRankPlayer();

    if (!pPlanter || !pPlanter->IsValidPlayer())
        return;

    pPlanter->m_Points.Add(g_CConfig->GetPointsWinBombPlantedPlayer());
    pPlanter->m_BombPlanted.Add(1);

    // Need to wait the next frame, either planter not receive the message
    new CTimer(0.0f, false, [pPlanterController]()
               {
        char szTranslate[256];

        UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("BOMB_PLANT_PLAYER"), g_CConfig->GetPointsWinBombPlantedPlayer());
        g_CChat->PrintToChat(pPlanterController, true, szTranslate);
        return -1.0f; });
}

GAME_EVENT_F(bomb_defused)
{
    if (!g_CConfig->IsMinimumPlayerReached())
        return;

    g_CPlayerManager->AddTeamPoint(CS_TEAM_CT, g_CConfig->GetPointsWinBombDefusedTeam());

    char szTranslate[256];
    UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("BOMB_DEFUSED_TEAM"), g_CConfig->GetPointsWinBombDefusedTeam());
    g_CChat->PrintToChatAll(szTranslate);

    // Defuser process

    CCSPlayerController *pDefuserController = (CCSPlayerController *)pEvent->GetPlayerController("userid");

    if (!pDefuserController)
        return;

    CRankPlayer *pDefuser = pDefuserController->GetRankPlayer();

    if (!pDefuser || !pDefuser->IsValidPlayer())
        return;

    pDefuser->m_Points.Add(g_CConfig->GetPointsWinBombDefusedPlayer());
    pDefuser->m_BombDefused.Add(1);

    // Need to wait the next frame, either defuser not receive the message
    new CTimer(0.0f, false, [pDefuserController]()
               {
        char szTranslate[256];

        UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("BOMB_DEFUSED_PLAYER"), g_CConfig->GetPointsWinBombDefusedPlayer());
        g_CChat->PrintToChat(pDefuserController, true, szTranslate);
        return -1.0f; });
}

GAME_EVENT_F(bomb_exploded)
{
    if (!g_CConfig->IsMinimumPlayerReached())
        return;

    g_CPlayerManager->AddTeamPoint(CS_TEAM_T, g_CConfig->GetPointsWinBombExplodedTeam());

    char szTranslate[256];
    UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("BOMB_EXPLODED_TEAM"), g_CConfig->GetPointsWinBombExplodedTeam());
    g_CChat->PrintToChatAll(szTranslate);

    // Planter process

    CCSPlayerController *pPlanterController = (CCSPlayerController *)pEvent->GetPlayerController("userid");

    if (!pPlanterController)
        return;

    CRankPlayer *pPlanter = pPlanterController->GetRankPlayer();

    if (!pPlanter || !pPlanter->IsValidPlayer())
        return;

    pPlanter->m_Points.Add(g_CConfig->GetPointsWinBombExplodedPlayer());
    pPlanter->m_BombExploded.Add(1);

    UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("BOMB_EXPLODED_PLAYER"), g_CConfig->GetPointsWinBombExplodedPlayer());
    g_CChat->PrintToChat(pPlanterController, true, szTranslate);
}

#ifdef _DEBUG

GAME_EVENT_F(player_spawn)
{
    CCSPlayerController *pController = (CCSPlayerController *)pEvent->GetPlayerController("userid");

    if (!pController)
        return;

    CHandle<CCSPlayerController> hController = pController->GetHandle();

    new CTimer(0.0f, false, [hController]()
               {
        CCSPlayerController *pController = hController.Get();

        if(!pController)
            return -1.0f;

        CBasePlayerPawn *pPawn = pController->m_hPawn();

        if(!pPawn)
            return -1.0f;
        
        CRankPlayer *pPlayer = pController->GetRankPlayer();

        if(!pPlayer || !pPlayer->IsFakeClient())
            return -1.0f;

        pPawn->m_MoveType = MOVETYPE_NONE;

		return -1.0f; });
}

#endif

GAME_EVENT_F(player_death)
{
    if (!g_CConfig->IsMinimumPlayerReached())
        return;

    CCSPlayerController *pVictimController = (CCSPlayerController *)pEvent->GetPlayerController("userid");
    CCSPlayerController *pAttackerController = (CCSPlayerController *)pEvent->GetPlayerController("attacker");

    if (!pVictimController || !pAttackerController)
        return;

    CRankPlayer *pVictim = pVictimController->GetRankPlayer();
    CRankPlayer *pAttacker = pAttackerController->GetRankPlayer();

    // Disable if invalid player
    if (!pVictim || !pAttacker || !pAttacker->IsValidPlayer())
        return;

    if (pVictim->IsFakeClient() && !g_CConfig->IsBotEnabled())
        return;

    char szTranslate[256];

    // Suicide
    if (pVictimController == pAttackerController)
    {
        // Using a timer, to be sure that the attacker is already valid and on the same team
        // We need this, because the suicide can occurs by moving on SPECTATOR or disconnecting

        int deathTeam = pAttackerController->m_iTeamNum;
        new CTimer(0.2f, false, [pAttackerController, deathTeam]()
                   {
                    if(!pAttackerController)
                        return -0.5f;

                    CRankPlayer *pAttacker = pAttackerController->GetRankPlayer();
                    
                    if(!pAttacker)
                        return -0.5f;

                    if(pAttackerController->m_iTeamNum != CS_TEAM_T && pAttackerController->m_iTeamNum != CS_TEAM_CT)
                        return -0.5f;

            pAttacker->m_Points.Remove(g_CConfig->GetPointsLooseSuicide());
            pAttacker->m_DeathSuicide.Add(1);

            char szTranslate[256];
            UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("DEATH_SUICIDE"), g_CConfig->GetPointsLooseSuicide());
            g_CChat->PrintToChat(pAttackerController, true, szTranslate);
        
        return -0.5f; });

        return;
    }

    if (pVictimController->m_iTeamNum == CS_TEAM_CT)
        pVictim->m_DeathCT.Add(1);
    else if (pVictimController->m_iTeamNum == CS_TEAM_T)
        pVictim->m_DeathT.Add(1);

    // Teamkill
    if (!g_CConfig->IsFFaEnabled() && pVictimController->m_iTeamNum.Get() == pAttackerController->m_iTeamNum.Get())
    {
        pAttacker->m_Points.Remove(g_CConfig->GetPointsLooseTeamkill());

        if (pAttackerController->m_iTeamNum == CS_TEAM_CT)
            pAttacker->m_TeamKillCT.Add(1);
        else if (pAttackerController->m_iTeamNum == CS_TEAM_T)
            pAttacker->m_TeamKillT.Add(1);

        UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("TEAMKILL"), g_CConfig->GetPointsLooseTeamkill());
        g_CChat->PrintToChat(pAttackerController, true, szTranslate);

        return;
    }

    const char *weapon = pEvent->GetString("weapon");

    if (pAttackerController->m_iTeamNum == CS_TEAM_CT)
        pAttacker->m_KillCT.Add(1);
    else if (pAttackerController->m_iTeamNum == CS_TEAM_T)
        pAttacker->m_KillT.Add(1);

    CCSPlayerController *pAssisterController = (CCSPlayerController *)pEvent->GetPlayerController("assister");
    if (pAssisterController)
    {
        CRankPlayer *pAssist = pAssisterController->GetRankPlayer();
        if (pAssist && pAssist->IsValidPlayer() && pAssisterController->m_iTeamNum != pVictimController->m_iTeamNum)
        {
            UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("KILL_ASSIST"), g_CConfig->GetPointsWinKillAssist());
            g_CChat->PrintToChat(pAssisterController, true, szTranslate);

            pAssist->m_Points.Add(g_CConfig->GetPointsWinKillAssist());

            if (pAssisterController->m_iTeamNum == CS_TEAM_T)
                pAssist->m_KillAssistCT.Add(1);
            else if (pAssisterController->m_iTeamNum == CS_TEAM_CT)
                pAssist->m_KillAssistT.Add(1);
        }
    }

    if (strstr(weapon, "knife") != nullptr)
    {
        pAttacker->m_Points.Add(g_CConfig->GetPointsWinKillKnife());
        pAttacker->m_KillKnife.Add(1);

        UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("KILL_KNIFE"), g_CConfig->GetPointsWinKillKnife());
        g_CChat->PrintToChat(pAttackerController, true, szTranslate);

        if (!pVictim->IsFakeClient())
        {
            pVictim->m_Points.Remove(g_CConfig->GetPointsLooseKillKnife());

            UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("DEATH_BY_KNIFE"), g_CConfig->GetPointsLooseKillKnife());
            g_CChat->PrintToChat(pVictimController, true, szTranslate);
        }

        return;
    }

    bool bHeadshot = pEvent->GetBool("headshot");

    if (bHeadshot)
    {
        pAttacker->m_Points.Add(g_CConfig->GetPointsWinKillWeaponHs());
        pAttacker->m_KillHeadshot.Add(1);

        UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("KILL_HEADSHOT"), g_CConfig->GetPointsWinKillWeaponHs());
        g_CChat->PrintToChat(pAttackerController, true, szTranslate);

        if (!pVictim->IsFakeClient())
        {
            pVictim->m_Points.Remove(g_CConfig->GetPointsLooseKillWeaponHs());

            UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("DEATH_BY_HEADSHOT"), g_CConfig->GetPointsLooseKillWeaponHs());
            g_CChat->PrintToChat(pVictimController, true, szTranslate);
        }
    }
    else
    {
        pAttacker->m_Points.Add(g_CConfig->GetPointsWinKillWeapon());

        UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("KILL_WEAPON"), g_CConfig->GetPointsWinKillWeapon());
        g_CChat->PrintToChat(pAttackerController, true, szTranslate);

        if (!pVictim->IsFakeClient())
        {
            pVictim->m_Points.Remove(g_CConfig->GetPointsLooseKillWeapon());

            UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("DEATH_BY_WEAPON"), g_CConfig->GetPointsLooseKillWeapon());
            g_CChat->PrintToChat(pVictimController, true, szTranslate);
        }
    }
}