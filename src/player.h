#pragma once

#include "abstract.h"
#include "mysql.h"
#include "steam/steamclientpublic.h"

extern CGlobalVars *g_pGlobals;

class CRankPlayerStats
{
public:
    CRankPlayerStats()
    {
    }

    int Get(RequestType requestType)
    {
        switch (requestType)
        {
        case RequestType::Global:
            return this->m_iGlobal;
        case RequestType::Map:
            return this->m_iMap;
        case RequestType::Session:
            return this->m_iSession;
        }
    }

    void Set(RequestType requestType, int value)
    {
        switch (requestType)
        {
        case RequestType::Global:
            this->m_iGlobal = value;
            break;
        case RequestType::Map:
            this->m_iMap = value;
            break;
        case RequestType::Session:
            this->m_iSession = value;
            break;
        }
    }

    void Reset(RequestType requestType)
    {
        switch (requestType)
        {
        case RequestType::Global:
            this->m_iGlobal = 0;
            break;
        case RequestType::Map:
            this->m_iMap = 0;
            break;
        }
    }

    void Add(int value)
    {
        this->m_iGlobal = this->m_iGlobal + value;
        this->m_iMap = this->m_iMap + value;
        this->m_iSession = this->m_iSession + value;
    }

    void Remove(int value)
    {
        this->m_iGlobal = this->m_iGlobal - value;
        this->m_iMap = this->m_iMap - value;
        this->m_iSession = this->m_iSession - value;
    }

private:
    int m_iGlobal;
    int m_iMap;
    int m_iSession;
};

class CRankPlayer
{
public:
    CRankPlayer(CPlayerSlot slot, bool m_bFakeClient = false) : m_slot(slot), m_bFakeClient(m_bFakeClient)
    {
        m_bAuthenticated = false;
        m_SteamID = nullptr;
        m_bConnected = false;
        m_bDatabaseAuthenticated = false;
        m_bIgnoringAnnouce = false;

        m_Points.Set(RequestType::Session, 0);
        m_DeathSuicide.Set(RequestType::Session, 0);
        m_DeathT.Set(RequestType::Session, 0);
        m_DeathCT.Set(RequestType::Session, 0);
        m_BombPlanted.Set(RequestType::Session, 0);
        m_BombExploded.Set(RequestType::Session, 0);
        m_BombDefused.Set(RequestType::Session, 0);
        m_KillKnife.Set(RequestType::Session, 0);
        m_KillHeadshot.Set(RequestType::Session, 0);
        m_KillCT.Set(RequestType::Session, 0);
        m_KillT.Set(RequestType::Session, 0);
        m_KillAssistCT.Set(RequestType::Session, 0);
        m_KillAssistT.Set(RequestType::Session, 0);
        m_TeamKillT.Set(RequestType::Session, 0);
        m_TeamKillCT.Set(RequestType::Session, 0);
    }

    CRankPlayer *Get() { return this; };
    void Reset(RequestType requestType);
    void InitStats(RequestType requestType, bool setAnnouce);

    void PrintDebug(RequestType requestType);

    bool IsFlooding();

    bool IsValidPlayer();

    void SaveOnDatabase();
    void RemoveFromOtherMap();

    bool IsDatabaseAuthenticated() { return m_bDatabaseAuthenticated; }
    void SetDatabaseAuthenticated() { m_bDatabaseAuthenticated = true; }

    bool IsFakeClient() { return m_bFakeClient; }

    bool IsAuthenticated() { return m_bAuthenticated; }
    void SetAuthenticated() { m_bAuthenticated = true; }

    bool IsConnected() { return m_bConnected; }
    void SetConnected() { m_bConnected = true; }

    void SetSteamId(const CSteamID *steamID) { m_SteamID = steamID; }
    uint64 GetSteamId64() { return m_SteamID->ConvertToUint64(); }
    const CSteamID *GetSteamId() { return m_SteamID; }

    void SetPlayerSlot(CPlayerSlot slot) { m_slot = slot; }
    CPlayerSlot GetPlayerSlot() { return m_slot; }

    bool IsIgnoringAnnouce() { return m_bIgnoringAnnouce; }
    void SetIgnoringAnnouce(bool value) { m_bIgnoringAnnouce = value; }

    CRankPlayerStats m_Points;
    CRankPlayerStats m_DeathSuicide;
    CRankPlayerStats m_DeathT;
    CRankPlayerStats m_DeathCT;
    CRankPlayerStats m_BombPlanted;
    CRankPlayerStats m_BombExploded;
    CRankPlayerStats m_BombDefused;
    CRankPlayerStats m_KillKnife;
    CRankPlayerStats m_KillHeadshot;
    CRankPlayerStats m_KillCT;
    CRankPlayerStats m_KillT;
    CRankPlayerStats m_KillAssistCT;
    CRankPlayerStats m_KillAssistT;
    CRankPlayerStats m_TeamKillT;
    CRankPlayerStats m_TeamKillCT;

private:
    const CSteamID *m_SteamID;
    CPlayerSlot m_slot;

    bool m_bIgnoringAnnouce;
    bool m_bAuthenticated;
    bool m_bConnected;
    bool m_bFakeClient;
    bool m_bDatabaseAuthenticated;

    int m_iFloodTokens;
    float m_flLastTalkTime;
};

class CPlayerManager
{
public:
    CPlayerManager()
    {
        V_memset(m_vecPlayers, 0, sizeof(m_vecPlayers));
    }

    bool OnClientConnected(CPlayerSlot slot);
    void OnClientDisconnect(CPlayerSlot slot);
    void OnBotConnected(CPlayerSlot slot);
    void OnLateLoad();
    void TryAuthenticate();

    void AddTeamPoint(int teamNumb, int point);
    void SaveAll();

    CRankPlayer *GetPlayer(CPlayerSlot slot);

private:
    CRankPlayer *m_vecPlayers[MAXPLAYERS];
};

extern CPlayerManager *g_CPlayerManager;