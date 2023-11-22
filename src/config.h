#include <string>
#include "KeyValues.h"
#include <ctime>
#include "abstract.h"
#include <vector>

extern IFileSystem *g_pFullFileSystem;

struct Phrase
{
    std::string key;
    std::string translation;
};

class CConfig
{
public:
    CConfig()
    {
    }

    bool Init(char *conf_error, int conf_error_size);
    void Destroy();

    const char *Translate(const std::string &key);

    int GetMysqlPort() { return g_iMysqlPort; }
    const char *GetMysqlHost() { return g_pszMysqlHost; }
    const char *GetMysqlDatabase() { return g_pszMysqlDatabase; }
    const char *GetMysqlPassword() { return g_pszMysqlPassword; }
    const char *GetMysqlUser() { return g_pszMysqlUser; }

    const char *GetRankReference() { return g_pszRankReference; }

    bool IsMinimumPlayerReached();
    bool IsFFaEnabled() { return g_bIsFFAEnabled; }
    bool IsNegativePointsAllowed() { return g_bIsNegativePointsAllowed; }
    bool IsRankAllowed() { return g_bIsResetRankAllowed; }

    int GetMinimumPoints() { return g_iMinimumPoints <= 0 ? 10 : g_iMinimumPoints; }
    int GetMinimumSessionPoints() { return g_iMinimumSessionPoints <= 0 ? 10 : g_iMinimumSessionPoints; }
    int GetMinimumPlayers() { return g_iMinimumPlayers <= 0 ? 5 : g_iMinimumPlayers; }

    int GetPointsLooseSuicide() { return g_iPointsLooseSuicide; }
    int GetPointsLooseTeamkill() { return g_iPointsLooseTeamkill; }
    int GetPointsLooseKillWeapon() { return g_iPointsLooseKillWeapon; }
    int GetPointsLooseKillWeaponHs() { return g_iPointsLooseKillWeaponHs; }
    int GetPointsLooseKillKnife() { return g_iPointsLooseKillKnife; }
    int GetPointsWinKillWeapon() { return g_iPointsWinKillWeapon; }
    int GetPointsWinKillWeaponHs() { return g_iPointsWinKillWeaponHs; }
    int GetPointsWinKillKnife() { return g_iPointsWinKillKnife; }
    int GetPointsWinBombPlantedPlayer() { return g_iPointsWinBombPlantedPlayer; }
    int GetPointsWinBombPlantedTeam() { return g_iPointsWinBombPlantedTeam; }
    int GetPointsWinBombExplodedPlayer() { return g_iPointsWinBombExplodedPlayer; }
    int GetPointsWinBombExplodedTeam() { return g_iPointsWinBombExplodedTeam; }
    int GetPointsWinBombDefusedPlayer() { return g_iPointsWinBombDefusedPlayer; }
    int GetPointsWinBombDefusedTeam() { return g_iPointsWinBombDefusedTeam; }
    int GetPointsWinKillAssist() { return g_iPointWinKillAssist; }

private:
    const char *g_pszMysqlHost;
    const char *g_pszMysqlUser;
    const char *g_pszMysqlDatabase;
    const char *g_pszMysqlPassword;
    int g_iMysqlPort;

    const char *g_pszRankReference;
    int g_iMinimumPlayers;
    int g_iMinimumPoints;
    int g_iMinimumSessionPoints;

    bool g_bIsFFAEnabled;
    bool g_bIsNegativePointsAllowed;
    bool g_bIsResetRankAllowed;
    bool g_bIsBotEnabled;
    
    const char *g_pszLanguage;
    std::vector<Phrase> g_vecPhrases;

    int g_iPointWinKillAssist;
    int g_iPointsLooseSuicide;
    int g_iPointsLooseTeamkill;
    int g_iPointsLooseKillWeapon;
    int g_iPointsLooseKillWeaponHs;
    int g_iPointsLooseKillKnife;
    int g_iPointsWinKillWeapon;
    int g_iPointsWinKillWeaponHs;
    int g_iPointsWinKillKnife;
    int g_iPointsWinBombPlantedPlayer;
    int g_iPointsWinBombPlantedTeam;
    int g_iPointsWinBombExplodedPlayer;
    int g_iPointsWinBombExplodedTeam;
    int g_iPointsWinBombDefusedPlayer;
    int g_iPointsWinBombDefusedTeam;

    KeyValues *g_kvCore;
    KeyValues *g_kvPoints;
    KeyValues *g_kvPhrases;
};

extern CConfig *g_CConfig;