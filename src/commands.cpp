#include "abstract.h"
#include "basecommands.h"
#include "chat.h"
#include "entity/ccsplayercontroller.h"
#include <map>
#include "mysql.h"
#include "config.h"
#include "metamod_util.h"
#include <algorithm>
#include "player.h"

void Print_TopPlayer(CPlayerSlot slot, std::map<std::string, int> players, RequestType requestType);
void Print_TopPlayerDatabaseWrapper(CCSPlayerController *player, RequestType requestType);

void Print_Rank(CPlayerSlot slot, int rank, RequestType requestType);
void Print_RankDatabaseWrapper(CCSPlayerController *player, RequestType requestType);

void Print_Stats(CPlayerSlot slot, RequestType requestType);
void Print_StatsDatabaseWrapper(CCSPlayerController *player, RequestType requestType);

std::map<std::string, int> GetMappedPlayer(int max, RequestType requestType, int minimumKill);
double CalcPercent(int want, int total);

// command to get all the cmds
CON_COMMAND_CHAT(rankh, "Display rank commands")
{
    int slot = player->GetPlayerSlot();
    CRankPlayer *pPlayer = player->GetRankPlayer();

    if (!pPlayer)
        return;

    char szBuffer[256];

    for (int i = 0; i < g_Commands.Count(); i++)
    {
        CChatCommand *value = g_Commands[i];
        const char *name = value->GetName();
        const char *description = value->GetDescription();

        UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("PRINT_COMMAND"), name, description);
        g_CChat->PrintToChat(slot, false, szBuffer);
    }
}

// enable or disable rank annoucement
CON_COMMAND_CHAT(rankannouce, "Ignore rank annoucement")
{
    int slot = player->GetPlayerSlot();
    CRankPlayer *pPlayer = player->GetRankPlayer();

    if (!pPlayer)
        return;

    if (pPlayer->IsIgnoringAnnouce())
    {
        pPlayer->SetIgnoringAnnouce(false);
        g_CChat->PrintToChat(slot, false, g_CConfig->Translate("IGNORE_ANNOUCE_ENABLE"));
    }
    else
    {
        pPlayer->SetIgnoringAnnouce(true);
        g_CChat->PrintToChat(slot, false, g_CConfig->Translate("IGNORE_ANNOUCE_DESABLE"));
    }
}

// command to display the rank
CON_COMMAND_CHAT(rank, "Display your rank")
{
    Print_RankDatabaseWrapper(player, RequestType::Global);
}

// command to display the rank
CON_COMMAND_CHAT(rankmap, "Display your rank for the map")
{
    Print_RankDatabaseWrapper(player, RequestType::Map);
}

// command to get the rank session
CON_COMMAND_CHAT(ranksession, "Display your rank for the current session")
{
    if (!player)
        return;

    int slot = player->GetPlayerSlot();
    CRankPlayer *pPlayer = player->GetRankPlayer();

    int minimumPoints = g_CConfig->GetMinimumSessionPoints();
    int playerPoints = pPlayer->m_Points.Get(RequestType::Session);

    if (playerPoints < minimumPoints)
    {
        int missingPoints = minimumPoints - playerPoints;

        char szBuffer[256];
        UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("MISSING_POINTS"), missingPoints);

        g_CChat->PrintToChat(slot, false, szBuffer);
        return;
    }

    std::map<std::string, int> players = GetMappedPlayer(g_pGlobals->maxClients, RequestType::Session, g_CConfig->GetMinimumSessionPoints());

    std::vector<std::pair<std::string, int>> playerVector(players.begin(), players.end());
    std::sort(playerVector.begin(), playerVector.end(), [](const auto &a, const auto &b)
              { return a.second > b.second; });

    const char *szName = player->GetPlayerName();
    auto it = std::find_if(playerVector.begin(), playerVector.end(), [szName](const std::pair<std::string, int> &element)
                           { return element.first == szName; });

    int rank = (it != playerVector.end()) ? std::distance(playerVector.begin(), it) : -1;

    Print_Rank(pPlayer->GetPlayerSlot(), rank + 1, RequestType::Session);
}

// top command
CON_COMMAND_CHAT(top, "Display the top players")
{
    Print_TopPlayerDatabaseWrapper(player, RequestType::Global);
}

// top command
CON_COMMAND_CHAT(topmap, "Display the top players for the map")
{
    Print_TopPlayerDatabaseWrapper(player, RequestType::Map);
}

// top sessions cmd
CON_COMMAND_CHAT(topsession, "Display the top players for the current session")
{
    if (!player)
        return;

    std::map<std::string, int> players = GetMappedPlayer(14, RequestType::Session, g_CConfig->GetMinimumSessionPoints());
    Print_TopPlayer(player->GetPlayerSlot(), players, RequestType::Session);
}

// command to reset rank
CON_COMMAND_CHAT(resetrank, "Reset your global rank")
{
    if (!player)
        return;

    int slot = player->GetPlayerSlot();
    CRankPlayer *pPlayer = g_CPlayerManager->GetPlayer(slot);

    if (!pPlayer || !pPlayer->IsDatabaseAuthenticated())
    {
        g_CChat->PrintToChat(slot, false, "%s", g_CConfig->Translate("NO_DB_AUTHICATED"));
        return;
    }

    pPlayer->Reset(RequestType::Global);
    g_CChat->PrintToChat(player, false, g_CConfig->Translate("RANK_RESET"));
}

// command to reset rank
CON_COMMAND_CHAT(resetmaprank, "Reset your map rank")
{
    if (!player)
        return;

    int slot = player->GetPlayerSlot();
    CRankPlayer *pPlayer = g_CPlayerManager->GetPlayer(slot);

    if (!pPlayer || !pPlayer->IsDatabaseAuthenticated())
    {
        g_CChat->PrintToChat(slot, false, "%s", g_CConfig->Translate("NO_DB_AUTHICATED"));
        return;
    }

    pPlayer->Reset(RequestType::Map);
    g_CChat->PrintToChat(player, false, g_CConfig->Translate("RANK_RESET_MAP"));
}

// command to show the stats
CON_COMMAND_CHAT(stats, "Show your stats")
{
    Print_StatsDatabaseWrapper(player, RequestType::Global);
}

// command to show the stats
CON_COMMAND_CHAT(statsmap, "Show your stats for the map")
{
    Print_StatsDatabaseWrapper(player, RequestType::Map);
}

// command to show the stats
CON_COMMAND_CHAT(statssession, "Show your stats")
{
    if (!player)
        return;

    int slot = player->GetPlayerSlot();

    Print_Stats(slot, RequestType::Session);
}

void Print_StatsDatabaseWrapper(CCSPlayerController *player, RequestType requestType)
{
    if (!player)
        return;

    int slot = player->GetPlayerSlot();
    CRankPlayer *pPlayer = g_CPlayerManager->GetPlayer(slot);

    if (!pPlayer || !pPlayer->IsDatabaseAuthenticated())
    {
        g_CChat->PrintToChat(slot, false, "%s", g_CConfig->Translate("NO_DB_AUTHICATED"));
        return;
    }

    Print_Stats(slot, requestType);
}

void Print_RankDatabaseWrapper(CCSPlayerController *player, RequestType requestType)
{
    if (!player)
        return;

    int slot = player->GetPlayerSlot();
    CRankPlayer *pPlayer = player->GetRankPlayer();

    if (!pPlayer || !pPlayer->IsDatabaseAuthenticated())
    {
        g_CChat->PrintToChat(slot, false, "%s", g_CConfig->Translate("NO_DB_AUTHICATED"));
        return;
    }

    int minimumPoints = g_CConfig->GetMinimumPoints();
    int playerPoints = pPlayer->m_Points.Get(requestType);

    if (playerPoints < minimumPoints)
    {
        int missingPoints = minimumPoints - playerPoints;

        char szBuffer[256];
        UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("MISSING_POINTS"), missingPoints);

        g_CChat->PrintToChat(slot, false, szBuffer);
        return;
    }

    g_CMysql->GetRank(requestType == RequestType::Global, pPlayer, [pPlayer, requestType](int rank)
                      { Print_Rank(pPlayer->GetPlayerSlot(), rank, requestType); });
}

// print the rank to a player slot, by checking if we need to display session message
void Print_Rank(CPlayerSlot slot, int rank, RequestType requestType)
{
    if (rank < 1)
    {
        g_CChat->PrintToChat(slot, false, g_CConfig->Translate("NOT_RANKED"));
        return;
    }

    char szBuffer[256];

    switch (requestType)
    {
    case RequestType::Global:
        UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("RANK"), rank);
        break;
    case RequestType::Map:
        UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("RANK_MAP"), rank);
        break;
    case RequestType::Session:
        UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("RANK_SESSION"), rank);
        break;
    }

    g_CChat->PrintToChat(slot, false, szBuffer, rank);
}

void Print_TopPlayerDatabaseWrapper(CCSPlayerController *player, RequestType requestType)
{
    if (!player)
        return;

    int slot = player->GetPlayerSlot();

    g_CMysql->GetTopPlayers(requestType == RequestType::Global, [slot, requestType](std::map<std::string, int> players)
                            { Print_TopPlayer(slot, players, requestType); });
}

// Print top player
void Print_TopPlayer(CPlayerSlot slot, std::map<std::string, int> players, RequestType requestType)
{
    if (players.empty())
    {
        g_CChat->PrintToChat(slot, false, "%s", g_CConfig->Translate("TOP_PLAYERS_NOTAVAILABLE"));
        return;
    }

    switch (requestType)
    {
    case RequestType::Global:
        g_CChat->PrintToChat(slot, false, "%s", g_CConfig->Translate("TOP_PLAYERS_TITLE"));
        break;
    case RequestType::Map:
        g_CChat->PrintToChat(slot, false, "%s", g_CConfig->Translate("TOP_PLAYERS_TITLE_MAP"));
        break;
    case RequestType::Session:
        g_CChat->PrintToChat(slot, false, "%s", g_CConfig->Translate("TOP_PLAYERS_TITLE_SESSION"));
        break;
    }

    // Sort, to be sure
    std::vector<std::pair<std::string, int>> playerVector(players.begin(), players.end());
    std::sort(playerVector.begin(), playerVector.end(), [](const auto &a, const auto &b)
              { return a.second > b.second; });

    int iteration = 1;
    char szPlayer[256];

    for (const auto &pair : playerVector)
    {
        UTIL_Format(szPlayer, sizeof(szPlayer), g_CConfig->Translate("TOP_PLAYER"), iteration, pair.first.c_str(), pair.second);
        g_CChat->PrintToChat(slot, false, "%s", szPlayer);

        iteration++;
    }
}

void Print_Stats(CPlayerSlot slot, RequestType requestType)
{
    CRankPlayer *pPlayer = g_CPlayerManager->GetPlayer(slot);

    if (!pPlayer)
        return;

    char szBuffer[256];

    switch (requestType)
    {
    case RequestType::Global:
        UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("STAT_TITLE"));
        break;
    case RequestType::Map:
        UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("STAT_MAP_TITLE"));
        break;
    case RequestType::Session:
        UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("STAT_SESSION_TITLE"));
        break;
    }

    g_CChat->PrintToChat(slot, false, szBuffer);

    UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("STAT_POINTS"), pPlayer->m_Points.Get(requestType));
    g_CChat->PrintToChat(slot, false, szBuffer);

    int totalKill = pPlayer->m_KillAssistCT.Get(requestType) + pPlayer->m_KillAssistT.Get(requestType);
    double percentKillT = CalcPercent(pPlayer->m_KillT.Get(requestType), totalKill);
    double percentKillCT = CalcPercent(pPlayer->m_KillCT.Get(requestType), totalKill);

    UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("STAT_KILL"), totalKill, percentKillT, percentKillCT);
    g_CChat->PrintToChat(slot, false, szBuffer);

    int totalDeath = pPlayer->m_DeathCT.Get(requestType) + pPlayer->m_DeathT.Get(requestType);
    double percentDeathT = CalcPercent(pPlayer->m_DeathT.Get(requestType), totalDeath);
    double percentDeathCT = CalcPercent(pPlayer->m_DeathCT.Get(requestType), totalDeath);

    UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("STAT_DEATH"), totalDeath, percentDeathT, percentDeathCT);
    g_CChat->PrintToChat(slot, false, szBuffer);

    int totalAssist = pPlayer->m_KillAssistCT.Get(requestType) + pPlayer->m_KillAssistT.Get(requestType);
    double parcentAssistT = CalcPercent(pPlayer->m_KillAssistT.Get(requestType), totalAssist);
    double parcentAssistCT = CalcPercent(pPlayer->m_KillAssistCT.Get(requestType), totalAssist);

    UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("STAT_ASSIST"), totalAssist, parcentAssistT, parcentAssistCT);
    g_CChat->PrintToChat(slot, false, szBuffer);

    int totalTeamKill = pPlayer->m_TeamKillCT.Get(requestType) + pPlayer->m_TeamKillT.Get(requestType);
    double percentTeamKillT = CalcPercent(pPlayer->m_TeamKillT.Get(requestType), totalTeamKill);
    double percentTeamKillCT = CalcPercent(pPlayer->m_TeamKillCT.Get(requestType), totalTeamKill);

    UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("STAT_TEAMKILL"), totalTeamKill, percentTeamKillT, percentTeamKillCT);
    g_CChat->PrintToChat(slot, false, szBuffer);

    double percentSuicide = CalcPercent(pPlayer->m_DeathSuicide.Get(requestType), totalDeath);

    UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("STAT_SUICIDE"), percentSuicide);
    g_CChat->PrintToChat(slot, false, szBuffer);

    double parcentHeadshot = CalcPercent(pPlayer->m_KillHeadshot.Get(requestType), totalKill);

    UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("STAT_HEADSHOT"), parcentHeadshot);
    g_CChat->PrintToChat(slot, false, szBuffer);

    double percentKnife = CalcPercent(pPlayer->m_KillKnife.Get(requestType), totalKill);

    UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("STAT_KNIFE"), percentKnife);
    g_CChat->PrintToChat(slot, false, szBuffer);
}

double CalcPercent(int want, int total)
{
    if (want <= 0 || total <= 0)
        return 0.0;

    return (static_cast<float>(want) / static_cast<float>(total)) * 100.0;
}

// get mapped player
std::map<std::string, int> GetMappedPlayer(int max, RequestType requestType, int minimumKill)
{
    std::map<std::string, int> players;

    for (int i = 0; i < g_pGlobals->maxClients; i++)
    {
        if (i > max)
            continue;

        CCSPlayerController *pController = CCSPlayerController::FromSlot(i);

        if (!pController)
            continue;

        CRankPlayer *pPlayerRank = pController->GetRankPlayer();
        if (!pPlayerRank || !pPlayerRank->IsValidPlayer() || pPlayerRank->m_Points.Get(requestType) < minimumKill)
            continue;

        const char *szName = pController->GetPlayerName();
        players[szName] = pPlayerRank->m_Points.Get(requestType);
    }

    return players;
}