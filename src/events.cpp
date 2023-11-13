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

    pPlanter->AddPoints(g_CConfig->GetPointsWinBombPlantedPlayer());
    pPlanter->AddBombPlanted(1);

    // Need to wait the next frame, either planter not receive the message
    new CTimer(0.0f, false, [pPlanterController]() {
        char szTranslate[256];

        UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("BOMB_PLANT_PLAYER"), g_CConfig->GetPointsWinBombPlantedPlayer());
        g_CChat->PrintToChat(pPlanterController, true, szTranslate);
        return -1.0f; 
    });
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

    pDefuser->AddPoints(g_CConfig->GetPointsWinBombDefusedPlayer());
    pDefuser->AddBombDefused(1);

    // Need to wait the next frame, either defuser not receive the message
    new CTimer(0.0f, false, [pDefuserController]() {
        char szTranslate[256];

        UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("BOMB_DEFUSED_PLAYER"), g_CConfig->GetPointsWinBombDefusedPlayer());
        g_CChat->PrintToChat(pDefuserController, true, szTranslate);
        return -1.0f; 
    });
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

    pPlanter->AddPoints(g_CConfig->GetPointsWinBombExplodedPlayer());
    pPlanter->AddBombExploded(1);

    UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("BOMB_EXPLODED_PLAYER"), g_CConfig->GetPointsWinBombExplodedPlayer());
    g_CChat->PrintToChat(pPlanterController, true, szTranslate);
}

GAME_EVENT_F(player_spawn)
{
    if (!g_CConfig->IsMinimumPlayerReached())
        return;

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

        if(!pPlayer->IsFakeClient())
            return -1.0f;

        pPawn->m_MoveType = MOVETYPE_NONE;

		return -1.0f; });
}

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

    // TODO enable, b'cause I have nobody to test with me :(
    // if (pVictim->IsFakeClient())
    // return;

    char szTranslate[256];

    // Suicide
    if (pVictimController == pAttackerController)
    {
        pAttacker->RemovePoints(g_CConfig->GetPointsLooseSuicide());
        pAttacker->AddDeathSuicide(1);

        UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("DEATH_SUICIDE"), g_CConfig->GetPointsLooseSuicide());
        g_CChat->PrintToChat(pAttackerController, true, szTranslate);

        return;
    }

    if (pVictimController->m_iTeamNum == CS_TEAM_CT)
        pVictim->AddDeathCT(1);
    else if (pVictimController->m_iTeamNum == CS_TEAM_T)
        pVictim->AddDeathT(1);

    // Teamkill, TODO: try with a deathmatch type FFA, not taking account if it's enable
    // OK
    if (pVictimController->m_iTeamNum.Get() == pAttackerController->m_iTeamNum.Get())
    {
        pAttacker->RemovePoints(g_CConfig->GetPointsLooseTeamkill());

        if (pAttackerController->m_iTeamNum == CS_TEAM_CT)
            pAttacker->AddTeamKillCT(1);
        else if (pAttackerController->m_iTeamNum == CS_TEAM_T)
            pAttacker->AddTeamKillT(1);

        UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("TEAMKILL"), g_CConfig->GetPointsLooseTeamkill());
        g_CChat->PrintToChat(pAttackerController, true, szTranslate);

        return;
    }

    const char *weapon = pEvent->GetString("weapon");

    if (pAttackerController->m_iTeamNum == CS_TEAM_CT)
        pAttacker->AddKillCT(1);
    else if (pAttackerController->m_iTeamNum == CS_TEAM_T)
        pAttacker->AddKillT(1);

    CCSPlayerController *pAssisterController = (CCSPlayerController *)pEvent->GetPlayerController("assister");
    if (pAssisterController)
    {
        CRankPlayer *pAssist = pAttackerController->GetRankPlayer();
        if (pAssist && pAssist->IsValidPlayer() && pAssisterController->m_iTeamNum != pVictimController->m_iTeamNum)
        {
            UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("KILL_ASSIST"), g_CConfig->GetPointsWinKillAssist());
            g_CChat->PrintToChat(pAttackerController, true, szTranslate);

            pAttacker->AddPoints(g_CConfig->GetPointsWinKillAssist());

            if (pAssisterController->m_iTeamNum == CS_TEAM_T)
                pAttacker->AddKillAssistT(1);
            else if (pAssisterController->m_iTeamNum == CS_TEAM_CT)
                pAttacker->AddKillAssistCT(1);
        }
    }

    if (strstr(weapon, "knife") != nullptr)
    {
        pAttacker->AddPoints(g_CConfig->GetPointsWinKillKnife());
        pAttacker->AddKillKnife(1);

        UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("KILL_KNIFE"), g_CConfig->GetPointsWinKillKnife());
        g_CChat->PrintToChat(pAttackerController, true, szTranslate);

        if (!pVictim->IsFakeClient())
        {
            pVictim->RemovePoints(g_CConfig->GetPointsLooseKillKnife());

            UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("DEATH_BY_KNIFE"), g_CConfig->GetPointsLooseKillKnife());
            g_CChat->PrintToChat(pVictimController, true, szTranslate);
        }

        return;
    }

    bool bHeadshot = pEvent->GetBool("headshot");

    if (bHeadshot)
    {
        pAttacker->AddPoints(g_CConfig->GetPointsWinKillWeaponHs());
        pAttacker->AddKillHeadshot(1);

        UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("KILL_HEADSHOT"), g_CConfig->GetPointsWinKillWeaponHs());
        g_CChat->PrintToChat(pAttackerController, true, szTranslate);

        if (!pVictim->IsFakeClient())
        {
            pVictim->RemovePoints(g_CConfig->GetPointsLooseKillWeaponHs());

            UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("DEATH_BY_HEADSHOT"), g_CConfig->GetPointsLooseKillWeaponHs());
            g_CChat->PrintToChat(pVictimController, true, szTranslate);
        }
    }
    else
    {
        pAttacker->AddPoints(g_CConfig->GetPointsWinKillWeapon());

        UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("KILL_WEAPON"), g_CConfig->GetPointsWinKillWeapon());
        g_CChat->PrintToChat(pAttackerController, true, szTranslate);

        if (!pVictim->IsFakeClient())
        {
            pVictim->RemovePoints(g_CConfig->GetPointsLooseKillWeapon());

            UTIL_Format(szTranslate, sizeof(szTranslate), g_CConfig->Translate("DEATH_BY_WEAPON"), g_CConfig->GetPointsLooseKillWeapon());
            g_CChat->PrintToChat(pVictimController, true, szTranslate);
        }
    }
}