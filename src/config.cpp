#include <config.h>
#include "abstract.h"

bool CConfig::Init(char *conf_error, int conf_error_size)
{
    // Core

    g_kvCore = new KeyValues("Core");
    if (!g_kvCore->LoadFromFile(g_pFullFileSystem, CORE_CONFIG_PATH))
    {
        snprintf(conf_error, conf_error_size, "Failed to load core configuration file");
        return false;
    }

    g_pszMysqlHost = g_kvCore->GetString("host", nullptr);
    g_pszMysqlPassword = g_kvCore->GetString("password", nullptr);
    g_pszMysqlUser = g_kvCore->GetString("user", nullptr);
    g_pszMysqlDatabase = g_kvCore->GetString("database", nullptr);
    g_iMysqlPort = g_kvCore->GetInt("port", 3306);

    g_iMinimumPoints = g_kvCore->GetInt("minimum_points", 100);
    g_iMinimumSessionPoints = g_kvCore->GetInt("minimum_session_points", 15);

    // Points configurations

    g_kvPoints = new KeyValues("Points");

    if (!g_kvPoints->LoadFromFile(g_pFullFileSystem, POINTS_CONFIG_PATH))
    {
        snprintf(conf_error, conf_error_size, "Failed to load points configuration file");
        return false;
    }

    g_iPointsLooseSuicide = g_kvPoints->GetInt("points_loose_suicide", 2);
    g_iPointsLooseTeamkill = g_kvPoints->GetInt("points_loose_teamkill", 2);
    g_iPointsLooseKillWeapon = g_kvPoints->GetInt("points_loose_kill_weapon", 1);
    g_iPointsLooseKillWeaponHs = g_kvPoints->GetInt("points_loose_kill_weapon_hs", 2);
    g_iPointsLooseKillKnife = g_kvPoints->GetInt("points_loose_kill_knife", 3);
    g_iPointsWinKillWeapon = g_kvPoints->GetInt("points_win_kill_weapon", 2);
    g_iPointsWinKillWeaponHs = g_kvPoints->GetInt("points_win_kill_weapon_hs",3);
    g_iPointsWinKillKnife = g_kvPoints->GetInt("points_win_kill_knife", 3);
    g_iPointsWinBombPlantedPlayer = g_kvPoints->GetInt("points_win_bomb_planted_player", 2);
    g_iPointsWinBombPlantedTeam = g_kvPoints->GetInt("points_win_bomb_planted_team", 1);
    g_iPointsWinBombExplodedPlayer = g_kvPoints->GetInt("points_win_bomb_exploded_player", 2);
    g_iPointsWinBombExplodedTeam = g_kvPoints->GetInt("points_win_bomb_exploded_team", 1);
    g_iPointsWinBombDefusedPlayer = g_kvPoints->GetInt("points_win_bomb_defused_player", 2);
    g_iPointsWinBombDefusedTeam = g_kvPoints->GetInt("points_win_bomb_defused_team", 1);
    g_iPointWinKillAssist = g_kvPoints->GetInt("points_win_kill_assist", 1);
    
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

    return key.c_str();
}

void CConfig::Destroy()
{   
    if(g_kvPoints)
        delete g_kvPoints;
    
    if(g_kvPhrases)
        delete g_kvPhrases;
    
    if(g_kvCore)
        delete g_kvCore;
}