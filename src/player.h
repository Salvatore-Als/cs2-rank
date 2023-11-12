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
        m_bIgnoringAnnouce = false;

        m_iPoints_S = 0;
        m_iDeathSuicide_S = 0;
        m_iDeathT_S = 0;
        m_iDeathCT_S = 0;
        m_iBombPlanted_S = 0;
        m_iBombExploded_S = 0;
        m_iBombDefused_S = 0;
        m_iKillKnife_S = 0;
        m_iKillHeadshot_S = 0;
        m_iKillCT_S = 0;
        m_iKillT_S = 0;
        m_iKillAssistCT_S = 0;
        m_iKillAssistT_S = 0;
        m_iTeamKillT_S = 0;
        m_iTeamKillCT_S = 0;
    }

    CRankPlayer *Get() { return this; };
    void Reset();
    void InitStats(bool setAnnouce);

    void PrintDebug(bool session);

    bool IsFlooding();

    bool IsValidPlayer();

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

    bool IsIgnoringAnnouce() { return m_bIgnoringAnnouce; }
    void SetIgnoringAnnouce(bool value) { m_bIgnoringAnnouce = value; }

    int GetPoints(bool session = false) const { return session ? m_iPoints_S : m_iPoints; }
    void AddPoints(int value)
    {
        Add(m_iPoints, value);
        Add(m_iPoints_S, value);
    }
    void RemovePoints(int value)
    {
        Remove(m_iPoints, value);
        Remove(m_iPoints_S, value);
    }
    void SetPoints(int value, bool session = true)
    {
        Set(m_iPoints, value);

        if (session)
            Set(m_iPoints_S, value);
    }

    int GetDeathSuicide(bool session = false) const { return session ? m_iDeathSuicide_S : m_iDeathSuicide; }
    void SetDeathSuicide(int value, bool session = true)
    {
        Set(m_iDeathSuicide, value);

        if (session)
            Set(m_iDeathSuicide_S, value);
    }
    void AddDeathSuicide(int value)
    {
        Add(m_iDeathSuicide, value);
        Add(m_iDeathSuicide_S, value);
    }

    int GetDeathT(bool session = false) const { return session ? m_iDeathT_S : m_iDeathT; }
    void SetDeathT(int value, bool session = true)
    {
        Set(m_iDeathT, value);

        if (session)
            Set(m_iDeathT_S, value);
    }
    void AddDeathT(int value)
    {
        Add(m_iDeathT, value);
        Add(m_iDeathT_S, value);
    }

    int GetDeathCT(bool session = false) const { return session ? m_iDeathCT_S : m_iDeathCT; }
    void SetDeathCT(int value, bool session = true)
    {
        Set(m_iDeathCT, value);

        if (session)
            Set(m_iDeathCT_S, value);
    }
    void AddDeathCT(int value)
    {
        Add(m_iDeathCT, value);
        Add(m_iDeathCT_S, value);
    }

    int GetBombPlanted(bool session = false) const { return session ? m_iBombPlanted_S : m_iBombPlanted; }
    void SetBombPlanted(int value, bool session = true)
    {
        Set(m_iBombPlanted, value);

        if (session)
            Set(m_iBombPlanted_S, value);
    }
    void AddBombPlanted(int value)
    {
        Add(m_iBombPlanted, value);
        Add(m_iBombPlanted_S, value);
    }

    int GetBombExploded(bool session = false) const { return session ? m_iBombExploded_S : m_iBombExploded; }
    void SetBombExploded(int value, bool session = true)
    {
        Set(m_iBombExploded, value);

        if (session)
            Set(m_iBombExploded_S, value);
    }
    void AddBombExploded(int value)
    {
        Add(m_iBombExploded, value);
        Add(m_iBombExploded_S, value);
    }

    int GetBombDefused(bool session = false) const { return session ? m_iBombDefused_S : m_iBombDefused; }
    void SetBombDefused(int value, bool session = true)
    {
        Set(m_iBombDefused, value);

        if (session)
            Set(m_iBombDefused_S, value);
    }
    void AddBombDefused(int value)
    {
        Add(m_iBombDefused, value);
        Add(m_iBombDefused_S, value);
    }

    int GetKillKnife(bool session = false) const { return session ? m_iKillKnife_S : m_iKillKnife; }
    void SetKillKnife(int value, bool session = true)
    {
        Set(m_iKillKnife, value);

        if (session)
            Set(m_iKillKnife_S, value);
    }
    void AddKillKnife(int value)
    {
        Add(m_iKillKnife, value);
        Add(m_iKillKnife_S, value);
    }

    int GetKillHeadshot(bool session = false) const { return session ? m_iKillHeadshot_S : m_iKillHeadshot; }
    void SetKillHeadshot(int value, bool session = true)
    {
        Set(m_iKillHeadshot, value);

        if (session)
            Set(m_iKillHeadshot_S, value);
    }
    void AddKillHeadshot(int value)
    {
        Add(m_iKillHeadshot, value);
        Add(m_iKillHeadshot_S, value);
    }

    int GetKillCT(bool session = false) const { return session ? m_iKillCT_S : m_iKillCT; }
    void SetKillCT(int value, bool session = true)
    {
        Set(m_iKillCT, value);

        if (session)
            Set(m_iKillCT_S, value);
    }
    void AddKillCT(int value)
    {
        Add(m_iKillCT, value);
        Add(m_iKillCT_S, value);
    }

    int GetKillT(bool session = false) const { return session ? m_iKillT_S : m_iKillT; }
    void SetKillT(int value, bool session = true)
    {
        Set(m_iKillT, value);

        if (session)
            Set(m_iKillT_S, value);
    }
    void AddKillT(int value)
    {
        Add(m_iKillT, value);
        Add(m_iKillT_S, value);
    }

    int GetKillAssistCT(bool session = false) const { return session ? m_iKillAssistCT_S : m_iKillAssistCT; }
    void SetKillAssistCT(int value, bool session = true)
    {
        Set(m_iKillAssistCT, value);

        if (session)
            Set(m_iKillAssistCT_S, value);
    }
    void AddKillAssistCT(int value)
    {
        Add(m_iKillAssistCT, value);
        Add(m_iKillAssistCT_S, value);
    }

    int GetKillAssistT(bool session = false) const { return session ? m_iKillAssistT_S : m_iKillAssistT; }
    void SetKillAssistT(int value, bool session = true)
    {
        Set(m_iKillAssistT, value);

        if (session)
            Set(m_iKillAssistT_S, value);
    }
    void AddKillAssistT(int value)
    {
        Add(m_iKillAssistT, value);
        Add(m_iKillAssistT_S, value);
    }

    int GetTeamKillT(bool session = false) const { return session ? m_iTeamKillT_S : m_iTeamKillT; }
    void SetTeamKillT(int value, bool session = true)
    {
        Set(m_iTeamKillT, value);

        if (session)
            Set(m_iTeamKillT_S, value);
    }
    void AddTeamKillT(int value)
    {
        Add(m_iTeamKillT, value);
        Add(m_iTeamKillT_S, value);
    }

    int GetTeamKillCT(bool session = false) const { return session ? m_iTeamKillCT_S : m_iTeamKillCT; }
    void SetTeamKillCT(int value, bool session = true)
    {
        Set(m_iTeamKillCT, value);

        if (session)
            Set(m_iTeamKillCT_S, value);
    }
    void AddTeamKillCT(int value)
    {
        Add(m_iTeamKillCT, value);
        Add(m_iTeamKillCT_S, value);
    }

private:
    void Add(int &variable, int value) { variable += value; }
    void Set(int &variable, int value) { variable = value; }
    void Remove(int &variable, int value) { variable -= value; }

    bool m_bIgnoringAnnouce;
    bool m_bAuthenticated;
    bool m_bConnected;
    bool m_bFakeClient;
    bool m_bDatabaseAuthenticated;

    const CSteamID *m_SteamID;
    CPlayerSlot m_slot;

    // _S convention for SESSION !!!

    int m_iPoints;
    int m_iPoints_S;

    int m_iDeathSuicide;
    int m_iDeathSuicide_S;
    int m_iDeathT;
    int m_iDeathT_S;
    int m_iDeathCT;
    int m_iDeathCT_S;

    int m_iBombPlanted;
    int m_iBombPlanted_S;
    int m_iBombExploded;
    int m_iBombExploded_S;
    int m_iBombDefused;
    int m_iBombDefused_S;

    int m_iKillKnife;
    int m_iKillKnife_S;
    int m_iKillHeadshot;
    int m_iKillHeadshot_S;
    int m_iKillCT;
    int m_iKillCT_S;
    int m_iKillT;
    int m_iKillT_S;
    int m_iKillAssistCT;
    int m_iKillAssistCT_S;
    int m_iKillAssistT;
    int m_iKillAssistT_S;

    int m_iTeamKillT;
    int m_iTeamKillT_S;
    int m_iTeamKillCT;
    int m_iTeamKillCT_S;

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