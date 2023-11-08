#include <config.h>
#include "abstract.h"

bool CConfig::Init(char *conf_error, int conf_error_size)
{
    // Mysql configuration

    g_kvMysql = new KeyValues("Mysql");
    if (!g_kvMysql->LoadFromFile(g_pFullFileSystem, MYSQL_CONFIG_PATH))
    {
        snprintf(conf_error, conf_error_size, "Failed to load mysql configuration file");
        return false;
    }

    g_pszMysqlHost = g_kvMysql->GetString("host", nullptr);
    g_pszMysqlPassword = g_kvMysql->GetString("password", nullptr);
    g_pszMysqlUser = g_kvMysql->GetString("user", nullptr);
    g_pszMsqlDatabase = g_kvMysql->GetString("database", nullptr);
    g_iMysqlPort = g_kvMysql->GetInt("port", 3306);

    // Points configurations

    g_kvPoints = new KeyValues("Points");

    if (!g_kvPoints->LoadFromFile(g_pFullFileSystem, POINTS_CONFIG_PATH))
    {
        snprintf(conf_error, conf_error_size, "Failed to load points configuration file");
        return false;
    }

    g_iPointsLooseSuicide = g_kvPoints->GetInt("points_loose_suicide");
    g_iPointsLooseTeamkill = g_kvPoints->GetInt("points_loose_teamkill");
    g_iPointsLooseKillWeapon = g_kvPoints->GetInt("points_loose_kill_weapon");
    g_iPointsLooseKillWeaponHs = g_kvPoints->GetInt("points_loose_kill_weapon_hs");
    g_iPointsLooseKillKnife = g_kvPoints->GetInt("points_loose_kill_knife");
    g_iPointsWinKillWeapon = g_kvPoints->GetInt("points_win_kill_weapon");
    g_iPointsWinKillWeaponHs = g_kvPoints->GetInt("points_win_kill_weapon_hs");
    g_iPointsWinKillKnife = g_kvPoints->GetInt("points_win_kill_knife");
    g_iPointsWinBombPlantedPlayer = g_kvPoints->GetInt("points_win_bomb_planted_player");
    g_iPointsWinBombPlantedTeam = g_kvPoints->GetInt("points_win_bomb_planted_team");
    g_iPointsWinBombExplodedPlayer = g_kvPoints->GetInt("points_win_bomb_exploded_player");
    g_iPointsWinBombExplodedTeam = g_kvPoints->GetInt("points_win_bomb_exploded_team");
    g_iPointsWinBombDefusedPlayer = g_kvPoints->GetInt("points_win_bomb_defused_player");
    g_iPointsWinBombDefusedTeam = g_kvPoints->GetInt("points_win_bomb_defused_team");

    // Phrases configurations

    g_kvPhrases = new KeyValues("Phrases");
    if (!g_kvPhrases->LoadFromFile(g_pFullFileSystem, PHRASES_CONFIG_PATH))
    {
        snprintf(conf_error, conf_error_size, "Failed to load core configuration file");
        return false;
    }

    const KeyValues *kvList = g_kvPhrases->FindKey("list");
    if (!kvList)
    {
        snprintf(conf_error, conf_error_size, "Failed to load list from configuration fle");
        return false;
    }

    g_pszLanguage = g_kvPhrases->GetString("language", "en");

    for (KeyValues *pKey = kvList->GetFirstTrueSubKey(); pKey; pKey = pKey->GetNextTrueSubKey())
    {
        g_vecPhrases.push_back({pKey->GetName(), pKey->GetString(g_pszLanguage, pKey->GetName())});
    }

    return true;
}

 const char *CConfig::Translate(const std::string &key)
{
    for (const Phrase &phrase : g_vecPhrases)
    {
        if (phrase.key == key)
            return phrase.translation.c_str();
    }

    return "MISSING TRANSLATION";
}

void CConfig::Destroy()
{
    delete g_kvMysql;
    delete g_kvPoints;
    delete g_kvPhrases;
}