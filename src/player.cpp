#include "abstract.h"
#include "player.h"
#include "entity/ccsplayercontroller.h"
#include "mysql.h"
#include "config.h"
#include "tier0/memdbgon.h"

int SafeValue(int value)
{
    if (g_CConfig->IsNegativePointsAllowed())
        return value;

    if (value < 0)
    {
        Debug("Value is %i, returning 0", value);
        return 0;
    }

    return value;
}

bool CRankPlayer::IsValidPlayer()
{
    if (!this->IsConnected())
        return false;

    if (this->IsFakeClient())
        return false;

    if (!this->IsAuthenticated())
        return false;

    if (!this->IsDatabaseAuthenticated())
        return false;

    return true;
}

void CRankPlayer::SaveOnDatabase()
{
    // User is not authenticated from database
    if (!this->IsDatabaseAuthenticated())
        return;

    g_CMysql->UpdateUser(this->Get());
    Debug("Save player %lli on database", this->GetSteamId64());
}

void CRankPlayer::RemoveFromOtherMap()
{
    // User is not authenticated from database
    if (!this->IsDatabaseAuthenticated())
        return;

    g_CMysql->RemoveFromOtherMap(this->Get());
    Debug("Remove player %lli from other map on database", this->GetSteamId64());
}

void CRankPlayer::InitStats(RequestType requestType, bool setAnnouce)
{
    m_Points.Set(requestType, 0);
    m_DeathSuicide.Set(requestType, 0);
    m_DeathT.Set(requestType, 0);
    m_DeathCT.Set(requestType, 0);
    m_BombPlanted.Set(requestType, 0);
    m_BombExploded.Set(requestType, 0);
    m_BombDefused.Set(requestType, 0);
    m_KillKnife.Set(requestType, 0);
    m_KillHeadshot.Set(requestType, 0);
    m_KillCT.Set(requestType, 0);
    m_KillT.Set(requestType, 0);
    m_KillAssistCT.Set(requestType, 0);
    m_KillAssistT.Set(requestType, 0);
    m_TeamKillT.Set(requestType, 0);
    m_TeamKillCT.Set(requestType, 0);

    if (setAnnouce)
        this->SetIgnoringAnnouce(false);
}

void CRankPlayer::Reset(RequestType requestType)
{
    if (requestType == RequestType::Map)
    {
        this->InitStats(RequestType::Map, false);
        this->InitStats(RequestType::Session, false);
        this->SaveOnDatabase();
    }
    else if (requestType == RequestType::Global)
    {
        this->InitStats(RequestType::Global, false);
        this->InitStats(RequestType::Map, false);
        this->InitStats(RequestType::Session, false);
        this->SaveOnDatabase();
        this->RemoveFromOtherMap();
    }
}

bool CRankPlayer::IsFlooding()
{
    float time = g_pGlobals->curtime;
    float newTime = time + 0.75;

    if (m_flLastTalkTime >= time)
    {
        if (m_iFloodTokens >= 3)
        {
            m_flLastTalkTime = newTime + 5.0;
            return true;
        }
        else
        {
            m_iFloodTokens++;
        }
    }
    else if (m_iFloodTokens > 0)
    {
        m_iFloodTokens--;
    }

    m_flLastTalkTime = newTime;
    return false;
}

void CRankPlayer::PrintDebug(RequestType requestType)
{
    const char *type = "UNK";

    switch (requestType)
    {
    case RequestType::Global:
        type = "GLOBAL";
        break;
    case RequestType::Session:
        type = "SESSION";
        break;
    case RequestType::Map:
        type = "MAP";
        break;
    }

    Debug("- %s : Points %i", type, this->m_Points.Get(requestType));
    Debug("- %s : DeathSuicide %i", type, this->m_DeathSuicide.Get(requestType));
    Debug("- %s : DeathT %i", type, this->m_DeathT.Get(requestType));
    Debug("- %s : DeathCT %i", type, this->m_DeathCT.Get(requestType));
    Debug("- %s : BombPlanted %i", type, this->m_BombPlanted.Get(requestType));
    Debug("- %s : BombExploded %i", type, this->m_BombExploded.Get(requestType));
    Debug("- %s : BombDefused %i", type, this->m_BombDefused.Get(requestType));
    Debug("- %s : KillKnife %i", type, this->m_KillKnife.Get(requestType));
    Debug("- %s : KillHeadshot %i", type, this->m_KillHeadshot.Get(requestType));
    Debug("- %s : KillT %i", type, this->m_KillT.Get(requestType));
    Debug("- %s : KillCT %i", type, this->m_KillCT.Get(requestType));
    Debug("- %s : TeamKillT %i", type, this->m_TeamKillT.Get(requestType));
    Debug("- %s :GTeamKillCT %i", type, this->m_TeamKillCT.Get(requestType));
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
            continue;

        OnClientConnected(i);
    }
}

void CPlayerManager::TryAuthenticate()
{
    for (int i = 0; i < g_pGlobals->maxClients; i++)
    {
        if (m_vecPlayers[i] == nullptr)
            continue;

        if (m_vecPlayers[i]->IsFakeClient())
            continue;

        if (m_vecPlayers[i]->IsAuthenticated())
        {
            if (!m_vecPlayers[i]->IsDatabaseAuthenticated() && !m_vecPlayers[i]->IsDatabaseTryingAuthenticated() && g_CMysql->IsConnected())
                g_CMysql->GetUser(m_vecPlayers[i]->Get());

            continue;
        }

        if (g_pEngine->IsClientFullyAuthenticated(i))
        {
            m_vecPlayers[i]->SetAuthenticated();
            m_vecPlayers[i]->SetSteamId(g_pEngine->GetClientSteamID(i));
            // g_CMysql->GetUser(m_vecPlayers[i]->Get());
        }
    }
}

CRankPlayer *CPlayerManager::GetPlayer(CPlayerSlot slot)
{
    if (slot.Get() < 0 || slot.Get() >= g_pGlobals->maxClients)
        return nullptr;

    return m_vecPlayers[slot.Get()];
};

void CPlayerManager::AddTeamPoint(int team, int point)
{
    if (team != CS_TEAM_T && team != CS_TEAM_CT)
        return;

    for (int i = 0; i < g_pGlobals->maxClients; i++)
    {
        CCSPlayerController *pController = CCSPlayerController::FromSlot(i);

        if (!pController || pController->m_iTeamNum != team)
            continue;

        CRankPlayer *pPlayerT = pController->GetRankPlayer();
        if (!pPlayerT || !pPlayerT->IsValidPlayer())
            continue;

        pPlayerT->m_Points.Add(point);
    }
};

void CPlayerManager::SaveAll()
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