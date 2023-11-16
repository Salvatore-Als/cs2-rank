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

void Print_TopPlayer(CPlayerSlot slot, std::map<std::string, int> players, bool session);
void Print_Rank(CPlayerSlot slot, int rank, bool session);
void Print_Stats(CPlayerSlot slot, bool session);
std::map<std::string, int> GetMappedPlayer(int max, int minimumKill);
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
    int playerPoints = pPlayer->GetPoints(false);

    if (playerPoints < minimumPoints)
    {
        int missingPoints = minimumPoints - playerPoints;

        char szBuffer[256];
        UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("MISSING_POINTS"), missingPoints);

        g_CChat->PrintToChat(slot, false, szBuffer);
        return;
    }

    g_CMysql->GetRank(pPlayer, [pPlayer](int rank)
                      { Print_Rank(pPlayer->GetPlayerSlot(), rank, false); });
}

// command to get the rank session
CON_COMMAND_CHAT(ranksession, "Display your rank for the current session")
{
    if (!player)
        return;

    int slot = player->GetPlayerSlot();
    CRankPlayer *pPlayer = player->GetRankPlayer();

    int minimumPoints = g_CConfig->GetMinimumSessionPoints();
    int playerPoints = pPlayer->GetPoints(true);

    if (playerPoints < minimumPoints)
    {
        int missingPoints = minimumPoints - playerPoints;

        char szBuffer[256];
        UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("MISSING_POINTS"), missingPoints);

        g_CChat->PrintToChat(slot, false, szBuffer);
        return;
    }

    std::map<std::string, int> players = GetMappedPlayer(g_pGlobals->maxClients, g_CConfig->GetMinimumSessionPoints());

    std::vector<std::pair<std::string, int>> playerVector(players.begin(), players.end());
    std::sort(playerVector.begin(), playerVector.end(), [](const auto &a, const auto &b)
              { return a.second > b.second; });

    const char *szName = player->GetPlayerName();
    auto it = std::find_if(playerVector.begin(), playerVector.end(), [szName](const std::pair<std::string, int> &element)
                           { return element.first == szName; });

    int rank = (it != playerVector.end()) ? std::distance(playerVector.begin(), it) : -1;

    Print_Rank(pPlayer->GetPlayerSlot(), rank + 1, true);
}

// top command
CON_COMMAND_CHAT(top, "Display the top players")
{
    if (!player)
        return;

    int slot = player->GetPlayerSlot();

    g_CMysql->GetTopPlayers([slot](std::map<std::string, int> players)
                            { Print_TopPlayer(slot, players, false); });
}

// top sessions cmd
CON_COMMAND_CHAT(topsession, "Display the top players for the current session")
{
    if (!player)
        return;

    std::map<std::string, int> players = GetMappedPlayer(14, g_CConfig->GetMinimumSessionPoints());
    Print_TopPlayer(player->GetPlayerSlot(), players, true);
}

// command to reset rank
CON_COMMAND_CHAT(resetrank, "Reset your rank")
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

    pPlayer->Reset();
    g_CChat->PrintToChat(player, false, g_CConfig->Translate("RANK_RESET"));
}

// command to show the stats
CON_COMMAND_CHAT(stats, "Show your stats")
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

    Print_Stats(slot, false);
}

// command to show the stats
CON_COMMAND_CHAT(statssessions, "Show your stats")
{
    if (!player)
        return;

    int slot = player->GetPlayerSlot();

    Print_Stats(slot, true);
}

// print the rank to a player slot, by checking if we need to display session message
void Print_Rank(CPlayerSlot slot, int rank, bool session)
{
    if (rank < 1)
    {
        g_CChat->PrintToChat(slot, false, g_CConfig->Translate("NOT_RANKED"));
        return;
    }

    char szBuffer[256];
    UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate(session ? "RANK_SESSION" : "RANK"), rank);

    g_CChat->PrintToChat(slot, false, szBuffer, rank);
}

// Print top player
void Print_TopPlayer(CPlayerSlot slot, std::map<std::string, int> players, bool session)
{
    if (players.empty())
    {
        g_CChat->PrintToChat(slot, false, "%s", g_CConfig->Translate("TOP_PLAYERS_NOTAVAILABLE"));
        return;
    }

    g_CChat->PrintToChat(slot, false, "%s", g_CConfig->Translate(session ? "TOP_PLAYERS_TITLE_SESSION" : "TOP_PLAYERS_TITLE"));

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

void Print_Stats(CPlayerSlot slot, bool session)
{
    CRankPlayer *pPlayer = g_CPlayerManager->GetPlayer(slot);

    if (!pPlayer)
        return;

    char szBuffer[256];
    UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate(session ? "STAT_SESSION_TITLE" : "STAT_TITLE"), pPlayer->GetPoints(session));
    g_CChat->PrintToChat(slot, false, szBuffer);

    UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("STAT_POINTS"), pPlayer->GetPoints(session));
    g_CChat->PrintToChat(slot, false, szBuffer);

    int totalKill = pPlayer->GetKillCT(session) + pPlayer->GetKillT(session);
    double percentKillT = CalcPercent(pPlayer->GetKillT(session), totalKill);
    double percentKillCT = CalcPercent(pPlayer->GetKillCT(session), totalKill);

    UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("STAT_KILL"), totalKill, percentKillT, percentKillCT);
    g_CChat->PrintToChat(slot, false, szBuffer);

    int totalDeath = pPlayer->GetDeathCT(session) + pPlayer->GetDeathT(session);
    double percentDeathT = CalcPercent(pPlayer->GetDeathT(session), totalDeath);
    double percentDeathCT = CalcPercent(pPlayer->GetDeathCT(session), totalDeath);

    UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("STAT_DEATH"), totalDeath, percentDeathT, percentDeathCT);
    g_CChat->PrintToChat(slot, false, szBuffer);

    int totalAssist = pPlayer->GetKillAssistT(session) + pPlayer->GetKillAssistT(session);
    double parcentAssistT = CalcPercent(pPlayer->GetKillAssistT(session), totalAssist);
    double parcentAssistCT = CalcPercent(pPlayer->GetKillAssistCT(session), totalAssist);

    UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("STAT_ASSIST"), totalAssist, parcentAssistT, parcentAssistCT);
    g_CChat->PrintToChat(slot, false, szBuffer);

    int totalTeamKill = pPlayer->GetTeamKillCT(session) + pPlayer->GetTeamKillT(session);
    double percentTeamKillT = CalcPercent(pPlayer->GetTeamKillT(session), totalTeamKill);
    double percentTeamKillCT = CalcPercent(pPlayer->GetTeamKillCT(session), totalTeamKill);

    UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("STAT_TEAMKILL"), totalTeamKill, percentTeamKillT, percentTeamKillCT);
    g_CChat->PrintToChat(slot, false, szBuffer);

    double percentSuicide = CalcPercent(pPlayer->GetDeathSuicide(session), totalDeath);

    UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("STAT_SUICIDE"), percentSuicide);
    g_CChat->PrintToChat(slot, false, szBuffer);

    double parcentHeadshot = CalcPercent(pPlayer->GetKillHeadshot(session), totalKill);

    UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("STAT_HEADSHOT"), parcentHeadshot);
    g_CChat->PrintToChat(slot, false, szBuffer);

    double percentKnife = CalcPercent(pPlayer->GetKillKnife(session), totalKill);

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
std::map<std::string, int> GetMappedPlayer(int max, int minimumKill)
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
        if (!pPlayerRank || !pPlayerRank->IsValidPlayer() || pPlayerRank->GetPoints(true) < minimumKill)
            continue;

        const char *szName = pController->GetPlayerName();
        players[szName] = pPlayerRank->GetPoints(true);
    }

    return players;
}