#include "abstract.h"
#include "player.h"
#include "entity/ccsplayercontroller.h"
#include "mysql.h"

bool CRankPlayer::IsValidPlayer()
{
    if (!this->IsConnected())
    {
        return false;
    }

    //if (!this->IsFakeClient())
    //{
    //    return false;
    //}

    if (!this->IsAuthenticated())
    {
        return false;
    }

    if (!this->IsDatabaseAuthenticated())
    {
        return false;
    }

    return true;
}

void CRankPlayer::OnAuthenticated()
{
    g_CMysql->GetUser(this->Get());
}

void CRankPlayer::SaveOnDatabase()
{
    // User is not authenticated from database
    if (!this->IsDatabaseAuthenticated())
    {
        return;
    }

    g_CMysql->UpdateUser(this->Get());
    Debug("Save player %lli on database", this->GetSteamId64());
}

bool CPlayerManager::OnClientConnected(CPlayerSlot slot)
{
    Assert(m_vecPlayers[slot.Get()] == nullptr);

    CRankPlayer *pPlayer = new CRankPlayer(slot);
    pPlayer->SetConnected();
    m_vecPlayers[slot.Get()] = pPlayer;

    return true;
}

void CPlayerManager::OnBotConnected(CPlayerSlot slot)
{
    m_vecPlayers[slot.Get()] = new CRankPlayer(slot, true);
}

void CPlayerManager::OnClientDisconnect(CPlayerSlot slot)
{
    m_vecPlayers[slot.Get()]->SaveOnDatabase();

    delete m_vecPlayers[slot.Get()];
    m_vecPlayers[slot.Get()] = nullptr;
}

void CPlayerManager::OnLateLoad()
{
    for (int i = 0; i < g_pGlobals->maxClients; i++)
    {
        CCSPlayerController *pController = CCSPlayerController::FromSlot(i);

        if (!pController || !pController->IsController() || !pController->IsConnected())
        {
            continue;
        }

        OnClientConnected(i);
    }
}

void CPlayerManager::TryAuthenticate()
{
    for (int i = 0; i < g_pGlobals->maxClients; i++)
    {
        if (m_vecPlayers[i] == nullptr)
        {
            continue;
        }

        if (m_vecPlayers[i]->IsAuthenticated() || m_vecPlayers[i]->IsFakeClient())
        {
            continue;
        }

        if (g_pEngine->IsClientFullyAuthenticated(i))
        {
            m_vecPlayers[i]->SetAuthenticated();
            m_vecPlayers[i]->SetSteamId(g_pEngine->GetClientSteamID(i));
            m_vecPlayers[i]->OnAuthenticated();
        }
    }
}

CRankPlayer *CPlayerManager::GetPlayer(CPlayerSlot slot)
{
    if (slot.Get() < 0 || slot.Get() >= g_pGlobals->maxClients)
    {
        return nullptr;
    };

    return m_vecPlayers[slot.Get()];
};

void CPlayerManager::AddTeamPoint(int team, int point)
{
    if (team != CS_TEAM_T && team != CS_TEAM_CT)
    {
        return;
    }

    for (int i = 0; i < g_pGlobals->maxClients; i++)
    {
        CCSPlayerController *pController = CCSPlayerController::FromSlot(i);

        if (!pController || pController->m_iTeamNum != team)
        {
            continue;
        }

        CRankPlayer *pPlayerT = pController->GetRankPlayer();
        if (!pPlayerT || !pPlayerT->IsValidPlayer())
        {
            continue;
        }

        pPlayerT->SetPoints(pPlayerT->GetPoints() + point);
    }
};

void CPlayerManager::SaveAll()
{
    for (int i = 0; i < g_pGlobals->maxClients; i++)
    {
        CCSPlayerController *pController = CCSPlayerController::FromSlot(i);

        if (!pController)
        {
            continue;
        }

        CRankPlayer *pPlayer = pController->GetRankPlayer();
        //if (!pPlayer || !pPlayer->IsValidPlayer())
        if (!pPlayer)
        {
            continue;
        }

        pPlayer->SaveOnDatabase();
    }
}