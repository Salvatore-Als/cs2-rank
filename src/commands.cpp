#include "abstract.h"
#include "basecommands.h"
#include "chat.h"
#include "entity/ccsplayercontroller.h"
#include <map>
#include "mysql.h"
#include "config.h"
#include "metamod_util.h"
#include <algorithm>

void Print_TopPlayer(CPlayerSlot slot, std::map<std::string, int> players, bool session);
void Print_Rank(CPlayerSlot slot, int rank, bool session);
std::map<std::string, int> GetMappedPlayer(int max, int minimumKill);

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
        UTIL_Format(szPlayer, sizeof(szPlayer), g_CConfig->Translate("TOP_PLAYER"), iteration, pair.first, pair.second);
        g_CChat->PrintToChat(slot, false, "%s", szPlayer);

        iteration++;
    }
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