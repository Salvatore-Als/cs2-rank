#include "abstract.h"
#include "basecommands.h"
#include "chat.h"
#include "entity/ccsplayercontroller.h"
#include <map>
#include "mysql.h"
#include "config.h"

CON_COMMAND_CHAT(rank, "Display your rank")
{
    if (!player)
    {
        return;
    }

    g_CChat->PrintToChat(player, "Command works ");
}

CON_COMMAND_CHAT(sessions, "Display your rank")
{
    if (!player)
    {
        return;
    }

    g_CChat->PrintToChat(player, "Command works ");
}

CON_COMMAND_CHAT(stats, "Display your rank")
{
    if (!player)
    {
        return;
    }

    g_CChat->PrintToChat(player, "Command works ");
}

CON_COMMAND_CHAT(top, "Display your rank")
{
    if (!player)
    {
        return;
    }

    int slot = player->GetPlayerSlot();

    g_CMysql->GetTopPlayers([slot](std::map<std::string, int> players)
                            {
    g_CChat->PrintToChat(slot, "--- TOP PLAYERS ---");

    int iteration = 1;
    for (const auto& pair : players) {
            g_CChat->PrintToChat(slot, "%i - %s with %i point(s)", iteration, pair.first, pair.second);
            iteration++;
    } 
    
    g_CChat->PrintToChat(slot, "--- ----------- ---"); });
}

CON_COMMAND_CHAT(resetrank, "Reset your rank")
{
    if (!player)
    {
        return;
    }

    int slot = player->GetPlayerSlot();
    CRankPlayer *pPlayer = g_CPlayerManager->GetPlayer(slot);

    if (!pPlayer)
    {
        return;
    }

    pPlayer->Reset();
    g_CChat->PrintToChat(player, "Your rank has been reseted !");
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