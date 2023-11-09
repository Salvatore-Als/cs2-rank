#include "abstract.h"
#include "basecommands.h"
#include "chat.h"
#include "entity/ccsplayercontroller.h"
#include <map>
#include "mysql.h"
#include "config.h"
#include "metamod_util.h"
#include <algorithm>

void Callback_GetTotalPlayers(int slot, std::map<std::string, int> players);
void Callback_GetRank(CRankPlayer *pPlayer, int rank);

CON_COMMAND_CHAT(test, "Test")
{
    int slot = player->GetPlayerSlot();
    g_CChat->PrintToChat(slot, "{red}TEST {green}COLOR");
}

CON_COMMAND_CHAT(rank, "Display your rank")
{
    if (!player)
        return;

    int slot = player->GetPlayerSlot();
    CRankPlayer *pPlayer = player->GetRankPlayer();

    if (!pPlayer->IsDatabaseAuthenticated())
    {
        g_CChat->PrintToChat(slot, "%s", g_CConfig->Translate("NO_DB_AUTHICATED"));
        return;
    }

    int minimumPoints = g_CConfig->GetMinimumPoints();
    int playerPoints = pPlayer->GetPoints();

    if (playerPoints < minimumPoints)
    {
        int missingPoints = minimumPoints - playerPoints;

        char szBuffer[256];
        UTIL_Format(szBuffer, sizeof(szBuffer), g_CConfig->Translate("MISSING_POINTS"), missingPoints);

        g_CChat->PrintToChat(slot, szBuffer);
        return;
    }

    g_CMysql->GetRank(pPlayer, [pPlayer](int rank)
                      { Callback_GetRank(pPlayer, rank); });
}

void Callback_GetRank(CRankPlayer *pPlayer, int rank)
{
    g_CChat->PrintToChat(pPlayer->GetPlayerSlot(), "Rank is %i", rank);
}

CON_COMMAND_CHAT(top, "Shot the top players")
{

    int slot = player->GetPlayerSlot();

    g_CMysql->GetTopPlayers([slot](std::map<std::string, int> players)
                            { Callback_GetTotalPlayers(slot, players); });
}

void Callback_GetTotalPlayers(int slot, std::map<std::string, int> players)
{
    if (players.empty())
    {

        g_CChat->PrintToChat(slot, "%s", g_CConfig->Translate("TOP_PLAYERS_NOTAVAILABLE"));
        return;
    }

    g_CChat->PrintToChat(slot, "%s", g_CConfig->Translate("TOP_PLAYERS_TITLE"));

    // Sort, to be sure
    std::vector<std::pair<std::string, int>> playerVector(players.begin(), players.end());

    std::sort(playerVector.begin(), playerVector.end(), [](const auto &a, const auto &b)
              { return a.second > b.second; });

    int iteration = 1;
    char szPlayer[256];

    for (const auto &pair : playerVector)
    {
        UTIL_Format(szPlayer, sizeof(szPlayer), g_CConfig->Translate("TOP_PLAYER"), iteration, pair.first, pair.second);
        g_CChat->PrintToChat(slot, "%s", szPlayer);

        iteration++;
    }
}

CON_COMMAND_CHAT(resetrank, "Reset your rank")
{
    if (!player)
        return;

    int slot = player->GetPlayerSlot();
    CRankPlayer *pPlayer = g_CPlayerManager->GetPlayer(slot);

    if (!pPlayer)
        return;

    pPlayer->Reset();
    g_CChat->PrintToChat(player, g_CConfig->Translate("RANK_RESET"));
}