#pragma once

#include "abstract.h"
#include <vendor/mysql/include/mysql_mm.h>
#include "player.h"
#include <map>

#define CREATE_REFERENCES_TABLE "CREATE TABLE IF NOT EXISTS `verygames_rank_references` ( \
  `id` BIGINT(64) NOT NULL AUTO_INCREMENT, \
  `reference` varchar(32) NOT NULL, \
  `custom_name` varchar(64), \
  UNIQUE INDEX `id` (`id`) USING BTREE) \
  ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;"

#define SELECT_REFERENCE "SELECT `id` FROM `verygames_rank_references` WHERE `reference` = '%s'"

#define INSERT_REFERENCE "INSERT INTO `verygames_rank_references` (`reference`) VALUES ('%s');"

#define CREATE_USERS_TABLE "CREATE TABLE IF NOT EXISTS `verygames_rank_users` ( \
  `id` BIGINT(64) NOT NULL AUTO_INCREMENT, \
  `authid` BIGINT(64) NOT NULL DEFAULT '0', \
  `reference` varchar(32) NOT NULL, \
  `name` varchar(32) NOT NULL, \
  `ignore_annouce` INT(11) NOT NULL DEFAULT 0, \
  `points` int(11) NOT NULL DEFAULT 0, \
  `lastconnect` int(11) NOT NULL DEFAULT 0, \
  `death_suicide` int(11) NOT NULL DEFAULT 0, \
  `death_t` int(11) NOT NULL DEFAULT 0, \
  `death_ct` int(11) NOT NULL DEFAULT 0, \
  `bomb_planted` int(11) NOT NULL DEFAULT 0, \
  `bomb_exploded` int(11) NOT NULL DEFAULT 0, \
  `bomb_defused` int(11) NOT NULL DEFAULT 0, \
  `kill_knife` int(11) NOT NULL DEFAULT 0, \
  `kill_headshot` int(11) NOT NULL DEFAULT 0, \
  `kill_t` int(11) NOT NULL DEFAULT 0, \
  `kill_ct` int(11) NOT NULL DEFAULT 0, \
  `teamkill_ct` int(11) NOT NULL DEFAULT 0, \
  `teamkill_t` int(11) NOT NULL DEFAULT 0, \
  `killassist_t` int(11) NOT NULL DEFAULT 0, \
  `killassist_ct` int(11) NOT NULL DEFAULT 0, \
  UNIQUE INDEX `id` (`id`) USING BTREE) \
  ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;"

#define INSERT_USER "INSERT INTO `verygames_rank_users` (`authid`, `name`, `reference`) VALUES ('%lli', '%s', '%s');"

#define UPDATE_USER "UPDATE `verygames_rank_users` SET `name` = '%s', `ignore_annouce` = %d, \
`points` = %d, `death_suicide` = %d, `death_t` = %d, `death_ct` = %d, `bomb_planted` = %d, \
`bomb_exploded` = %d, `bomb_defused` = %d, `kill_knife` = %d, `kill_headshot` = %d, `kill_t` = %d, \
`kill_ct` = %d, `teamkill_t` = %d, `teamkill_ct` = %d, `lastconnect` = %d WHERE `authid` = '%lli' AND `reference` = '%s';"

#define SELECT_USER "SELECT `ignore_annouce`, `points`, `death_suicide`, `death_t`, `death_ct`, `bomb_planted`, `bomb_exploded` \
, `bomb_defused`, `kill_knife`, `kill_headshot`, `kill_t`, `kill_ct`, `teamkill_t` \
, `teamkill_ct`, `killassist_t`, `killassist_ct` FROM `verygames_rank_users` WHERE `authid` = '%lli' AND `reference` = '%s'"

#define TOP "SELECT `name`, `points` FROM verygames_rank_users WHERE points >= %i AND `reference` = '%s' ORDER BY points DESC LIMIT 15;"

#define RANK "SELECT COUNT(*) FROM `verygames_rank_users` WHERE `points` > %i AND `reference` = '%s';"

extern IVEngineServer2 *g_pEngine;
extern IMySQLClient *g_pMysqlClient;
extern IMySQLConnection *g_pConnection;

class CRankPlayer; // Todo: check to remove this shit

class CMysql
{
public:
  CMysql()
  {
    Connect();
  }

public:
  void Destroy();

  void UpdateUser(CRankPlayer *pPlayer);
  void GetUser(CRankPlayer *pPlayer);

  void GetTopPlayers(std::function<void(std::map<std::string, int>)> callback);
  void GetRank(CRankPlayer *pPlayer, std::function<void(int)> callback);

private:
  void Connect();
  void CreateDatabaseIfNotExist();

  void Query_GetRankReference(IMySQLQuery *cb);
  void Query_GetUser(IMySQLQuery *cb, CRankPlayer *pPlayer);
  void Query_TopPlayers(IMySQLQuery *cb, std::function<void(std::map<std::string, int>)> callback);
  void Query_Rank(IMySQLQuery *cb, std::function<void(int)> callback);
};

extern CMysql *g_CMysql;