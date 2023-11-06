#pragma once

#include "abstract.h"
#include "mysql.h"
#include "steam/steamclientpublic.h"

extern CGlobalVars *g_pGlobals;

class CRankPlayer
{
public:
    CRankPlayer(CPlayerSlot slot, bool m_bFakeClient = false) : m_slot(slot), m_bFakeClient(m_bFakeClient)
    {
        m_bAuthenticated = false;
        m_SteamID = nullptr;
        m_bConnected = false;
        m_bDatabaseAuthenticated = false;
    }

    CRankPlayer *Get() { return this; };

    bool IsValidPlayer();

    void OnAuthenticated();
    void SaveOnDatabase();

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

    int GetPoints() const { return m_iPoints; }
    void SetPoints(int value) { m_iPoints = value; }

    int GetDeathSuicide() const { return m_iDeathSuicide; }
    void SetDeathSuicide(int value) { m_iDeathSuicide = value; }

    int GetDeathT() const { return m_iDeathT; }
    void SetDeathT(int value) { m_iDeathT = value; }

    int GetDeathCT() const { return m_iDeathCT; }
    void SetDeathCT(int value) { m_iDeathCT = value; }

    int GetBombPlanted() const { return m_iBombPlanted; }
    void SetBombPlanted(int value) { m_iBombPlanted = value; }

    int GetBombExploded() const { return m_iBombExploded; }
    void SetBombExploded(int value) { m_iBombExploded = value; }

    int GetBombDefused() const { return m_iBombDefused; }
    void SetBombDefused(int value) { m_iBombDefused = value; }

    int GetKillKnife() const { return m_iKillKnife; }
    void SetKillKnife(int value) { m_iKillKnife = value; }

    int GetKillHeadshot() const { return m_iKillHeadshot; }
    void SetKillHeadshot(int value) { m_iKillHeadshot = value; }

    int GetKillCT() const { return m_iKillCT; }
    void SetKillCT(int value) { m_iKillCT = value; }

    int GetKillT() const { return m_iKillT; }
    void SetKillT(int value) { m_iKillT = value; }

    int GetTeamKillT() const { return m_iTeamKillT; }
    void SetTeamKillT(int value) { m_iTeamKillT = value; }

    int GetTeamKillCT() const { return m_iTeamKillCT; }
    void SetTeamKillCT(int value) { m_iTeamKillCT = value; }

private:
    bool m_bAuthenticated;
    bool m_bConnected;
    bool m_bFakeClient;
    bool m_bDatabaseAuthenticated;

    const CSteamID *m_SteamID;
    CPlayerSlot m_slot;

    int m_iPoints;

    int m_iDeathSuicide;
    int m_iDeathT;
    int m_iDeathCT;

    int m_iBombPlanted;
    int m_iBombExploded;
    int m_iBombDefused;

    int m_iKillKnife;
    int m_iKillHeadshot;
    int m_iKillCT;
    int m_iKillT;

    int m_iTeamKillT;
    int m_iTeamKillCT;
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