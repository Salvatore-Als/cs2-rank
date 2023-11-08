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

CON_COMMAND_CHAT(rank, "Display your rank")
{
    if (!player)
        return;

    int slot = player->GetPlayerSlot();
    CRankPlayer *pPlayer = player->GetRankPlayer();

    if (!pPlayer->IsDatabaseAuthenticated())
    {
        g_CChat->PrintToChat(slot, "You are not authenticated to our database");
        return;
    }

    g_CMysql->GetRank(pPlayer, [pPlayer](int rank)
                      { Callback_GetRank(pPlayer, rank); });
}

void Callback_GetRank(CRankPlayer *pPlayer, int rank)
{
    g_CChat->PrintToChat(pPlayer->GetPlayerSlot(), "Rank is %i", rank);
}

CON_COMMAND_CHAT(top, "Display your rank")
{

    int slot = player->GetPlayerSlot();

    g_CMysql->GetTopPlayers([slot](std::map<std::string, int> players)
                            { Callback_GetTotalPlayers(slot, players); });
}

void Callback_GetTotalPlayers(int slot, std::map<std::string, int> players)
{
    g_CChat->PrintToChat(slot, "%s", g_CConfig->Translate("TOP_PLAYERS_TITLE"));

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

CON_COMMAND_EXTERN(rank_debugtranslate, Command_DebugTranslate, "");
void Command_DebugTranslate(const CCommandContext &context, const CCommand &args)
{
    char translate1[256];
    UTIL_Format(translate1, sizeof(translate1), g_CConfig->Translate("TEST_TRANSLATE_1"), "String value", 15);

    char translate2[256];
    UTIL_Format(translate2, sizeof(translate2), g_CConfig->Translate("TEST_TRANSLATE_2"), "String value", 15);

    char translate3[256];
    UTIL_Format(translate3, sizeof(translate3), g_CConfig->Translate("NON_EXISTING"));

    g_CChat->PrintToServerConsole("PREFIX - %s %s %s %s", translate1, translate2, "string", translate3);
}

CON_COMMAND_EXTERN(rank_debugconfig, Command_DebugConfig, "");
void Command_DebugConfig(const CCommandContext &context, const CCommand &args)
{
    Debug("GetMysqlPort() : %i", g_CConfig->GetMysqlPort());
    Debug("GetMysqlHost() : %s", g_CConfig->GetMysqlHost());
    Debug("GetMysqlDatabase() : %s", g_CConfig->GetMysqlDatabase());
    Debug("GetMysqlDatabase() : %s", g_CConfig->GetMysqlPassword());
    Debug("GetMysqlUser() : %s", g_CConfig->GetMysqlUser());

    Debug("GetPointsLooseSuicide() : %i", g_CConfig->GetPointsLooseSuicide());
    Debug("GetPointsLooseTeamkill() : %i", g_CConfig->GetPointsLooseTeamkill());
    Debug("GetPointsLooseKillWeapon() : %i", g_CConfig->GetPointsLooseKillWeapon());
    Debug("GetPointsLooseKillWeaponHs() : %i", g_CConfig->GetPointsLooseKillWeaponHs());
    Debug("GetPointsLooseKillKnife() : %i", g_CConfig->GetPointsLooseKillKnife());
    Debug("GetPointsWinKillWeapon() : %i", g_CConfig->GetPointsWinKillWeapon());
    Debug("GetPointsWinKillWeaponHs() : %i", g_CConfig->GetPointsWinKillWeaponHs());
    Debug("GetPointsWinKillKnife() : %i", g_CConfig->GetPointsWinKillKnife());
    Debug("GetPointsWinBombPlantedPlayer() : %i", g_CConfig->GetPointsWinBombPlantedPlayer());
    Debug("GetPointsWinBombPlantedTeam() : %i", g_CConfig->GetPointsWinBombPlantedTeam());
    Debug("GetPointsWinBombExplodedPlayer() : %i", g_CConfig->GetPointsWinBombExplodedPlayer());
    Debug("GetPointsWinBombExplodedTeam() : %i", g_CConfig->GetPointsWinBombExplodedTeam());
    Debug("GetPointsWinBombDefusedPlayer() : %i", g_CConfig->GetPointsWinBombDefusedPlayer());
    Debug("GetPointsWinBombDefusedTeam() : %i", g_CConfig->GetPointsWinBombDefusedTeam());
}

CON_COMMAND_EXTERN(rank_debugsave, Command_RankSave, "");
void Command_RankSave(const CCommandContext &context, const CCommand &args)
{
    for (int i = 0; i < g_pGlobals->maxClients; i++)
    {
        CCSPlayerController *pController = CCSPlayerController::FromSlot(i);

        if (!pController)
        {
            continue;
        }

        CRankPlayer *pPlayer = pController->GetRankPlayer();
        if (!pPlayer || !pPlayer->IsValidPlayer())
        {
            continue;
        }

        pPlayer->SaveOnDatabase();
    }
}