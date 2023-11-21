#include "mysql.h"
#include "abstract.h"
#include <vendor/mysql/include/mysql_mm.h>
#include "player.h"
#include <map>
#include "config.h"
#include "main.h"
#include <ctime>

using namespace std;

void CMysql::Connect()
{
	if (!g_pMysqlClient)
		return;

	MySQLConnectionInfo info;
	info.host = g_CConfig->GetMysqlHost();
	info.user = g_CConfig->GetMysqlUser();
	info.pass = g_CConfig->GetMysqlPassword();
	info.database = g_CConfig->GetMysqlDatabase();
	info.port = g_CConfig->GetMysqlPort();

	g_pConnection = g_pMysqlClient->CreateMySQLConnection(info);

	g_pConnection->Connect([this](bool connect)
						   {
		if (!connect)
		{
			Fatal("Failed to connect the mysql database, unloading");
			g_CPlugin.ForceUnload();
		} else {
			//Debug("test escape string %s", g_pConnection->Escape("test").c_str());
			this->CreateDatabaseIfNotExist();
			this->m_bConnected = true;
		} });
}

void CMysql::Destroy()
{
	if (g_pConnection)
	{
		g_pConnection->Destroy();
		g_pConnection = nullptr;
	}
}

void CMysql::CreateDatabaseIfNotExist()
{
	if (!g_pConnection)
		return;

	g_pConnection->Query(CREATE_USERS_TABLE, [](IMySQLQuery *cb) {});
	g_pConnection->Query(CREATE_REFERENCES_TABLE, [](IMySQLQuery *cb) {});
	g_pConnection->Query(CREATE_MAPS_TABLE, [](IMySQLQuery *cb) {});

	Debug("Create Database request 1: %s", CREATE_USERS_TABLE);
	Debug("Create Database request 2: %s", CREATE_REFERENCES_TABLE);
	Debug("Create Database request 2: %s", CREATE_MAPS_TABLE);

	char szQuery[MAX_QUERY_SIZES];

	V_snprintf(szQuery, sizeof(szQuery), SELECT_REFERENCE, this->EscapeRankReference().c_str());
	g_pConnection->Query(szQuery, [this](IMySQLQuery *cb)
						 { this->Query_GetRankReference(cb); });

	
	Debug("Select Reference : %s", szQuery);
	
	V_snprintf(szQuery, sizeof(szQuery), SELECT_MAP, g_pConnection->Escape(g_pGlobals->mapname.ToCStr()).c_str());

	g_pConnection->Query(szQuery, [this](IMySQLQuery *cb)
						 { this->Query_GetMapId(cb); });

	Debug("Select Map Request : %s", szQuery);
}

void CMysql::Query_GetRankReference(IMySQLQuery *cb)
{
	IMySQLResult *results = cb->GetResultSet();

	if (!results)
	{
		Fatal("Invalid results for Query_GetUser");
		return;
	}

	if (results->FetchRow())
		return;

	char szQuery[MAX_QUERY_SIZES];
	V_snprintf(szQuery, sizeof(szQuery), INSERT_REFERENCE, this->EscapeRankReference().c_str());

	Debug("InsertReference request : %s", szQuery);

	g_pConnection->Query(szQuery, [](IMySQLQuery *cb) {});
}

void CMysql::Query_GetMapId(IMySQLQuery *cb)
{
	IMySQLResult *results = cb->GetResultSet();

	if (!results)
	{
		Fatal("Unable to get the map id, unloading");
		g_CPlugin.ForceUnload();
		return;
	}

	if (results->FetchRow())
	{
		this->g_iMapId = results->GetInt(0);
		return;
	}

	char szQuery[MAX_QUERY_SIZES];
	V_snprintf(szQuery, sizeof(szQuery), INSERT_MAP, g_pConnection->Escape(g_pGlobals->mapname.ToCStr()).c_str());

	Debug("InsertReference request : %s", szQuery);

	g_pConnection->Query(szQuery, [this](IMySQLQuery *cb)
						 { this->g_iMapId = cb->GetInsertId(); });
}

void CMysql::GetUser(CRankPlayer *pPlayer)
{
	if (!g_pConnection)
		return;

	pPlayer->InitStats(RequestType::Map, true);
	pPlayer->InitStats(RequestType::Session, true);
	pPlayer->InitStats(RequestType::Global, true);

	uint64 steamId64 = pPlayer->GetSteamId64();

	char szQuery[MAX_QUERY_SIZES];

	V_snprintf(szQuery, sizeof(szQuery), SELECT_USER_MAP, steamId64, this->EscapeRankReference().c_str(), this->g_iMapId);
	g_pConnection->Query(szQuery, [pPlayer, this](IMySQLQuery *cb)
						 { this->Query_GetUserMap(cb, pPlayer); });

	V_snprintf(szQuery, sizeof(szQuery), SELECT_USER_GLOBAL, steamId64, this->EscapeRankReference().c_str());
	g_pConnection->Query(szQuery, [pPlayer, this](IMySQLQuery *cb)
						 { this->Query_GetUserGlobal(cb, pPlayer); });

	Debug("GetUserMap Request : %s", szQuery);
	Debug("GetUserGlobal Request : %s", szQuery);
}

void CMysql::Query_GetUserMap(IMySQLQuery *cb, CRankPlayer *pPlayer)
{
	IMySQLResult *results = cb->GetResultSet();

	if (!results)
	{
		Fatal("Invalid results for Query_GetUserMap");
		return;
	}

	if (!results->FetchRow())
	{
		const char *name = g_pEngine->GetClientConVarValue(pPlayer->GetPlayerSlot(), "name");
		uint64 steamId64 = pPlayer->GetSteamId64();

		// NOTE : Only set the authid and name, because the other data have a default value set on the database schema

		char szQuery[MAX_QUERY_SIZES];
		V_snprintf(szQuery, sizeof(szQuery), INSERT_USER, steamId64, g_pConnection->Escape(name).c_str(), this->EscapeRankReference().c_str(), this->g_iMapId);

		Debug("InsertUser request : %s", szQuery);

		g_pConnection->Query(szQuery, [pPlayer](IMySQLQuery *cb)
							 { pPlayer->SetDatabaseAuthenticated(); });
	}
	else
	{
		pPlayer->SetDatabaseAuthenticated();

		Debug("User map points %i", results->GetInt(1));

		pPlayer->SetIgnoringAnnouce(results->GetInt(0) == 1 ? true : false);

		pPlayer->m_Points.Set(RequestType::Map, results->GetInt(1));
		pPlayer->m_DeathSuicide.Set(RequestType::Map, results->GetInt(2));
		pPlayer->m_DeathT.Set(RequestType::Map, results->GetInt(3));
		pPlayer->m_DeathCT.Set(RequestType::Map, results->GetInt(4));
		pPlayer->m_BombPlanted.Set(RequestType::Map, results->GetInt(5));
		pPlayer->m_BombExploded.Set(RequestType::Map, results->GetInt(6));
		pPlayer->m_BombDefused.Set(RequestType::Map, results->GetInt(7));
		pPlayer->m_KillKnife.Set(RequestType::Map, results->GetInt(8));
		pPlayer->m_KillHeadshot.Set(RequestType::Map, results->GetInt(9));
		pPlayer->m_KillT.Set(RequestType::Map, results->GetInt(10));
		pPlayer->m_KillCT.Set(RequestType::Map, results->GetInt(11));
		pPlayer->m_TeamKillT.Set(RequestType::Map, results->GetInt(12));
		pPlayer->m_TeamKillCT.Set(RequestType::Map, results->GetInt(13));
		pPlayer->m_KillAssistT.Set(RequestType::Map, results->GetInt(14));
		pPlayer->m_KillAssistCT.Set(RequestType::Map, results->GetInt(15));
	}
}

void CMysql::Query_GetUserGlobal(IMySQLQuery *cb, CRankPlayer *pPlayer)
{
	IMySQLResult *results = cb->GetResultSet();

	if (!results)
	{
		Fatal("Invalid results for Query_GetUserGlobal");
		return;
	}

	if (results->FetchRow())
	{
		Debug("User global points %i", results->GetInt(0));

		pPlayer->m_Points.Set(RequestType::Global, results->GetInt(0));
		pPlayer->m_DeathSuicide.Set(RequestType::Global, results->GetInt(1));
		pPlayer->m_DeathT.Set(RequestType::Global, results->GetInt(2));
		pPlayer->m_DeathCT.Set(RequestType::Global, results->GetInt(3));
		pPlayer->m_BombPlanted.Set(RequestType::Global, results->GetInt(4));
		pPlayer->m_BombExploded.Set(RequestType::Global, results->GetInt(5));
		pPlayer->m_BombDefused.Set(RequestType::Global, results->GetInt(6));
		pPlayer->m_KillKnife.Set(RequestType::Global, results->GetInt(7));
		pPlayer->m_KillHeadshot.Set(RequestType::Global, results->GetInt(8));
		pPlayer->m_KillT.Set(RequestType::Global, results->GetInt(9));
		pPlayer->m_KillCT.Set(RequestType::Global, results->GetInt(10));
		pPlayer->m_TeamKillT.Set(RequestType::Global, results->GetInt(11));
		pPlayer->m_TeamKillCT.Set(RequestType::Global, results->GetInt(12));
		pPlayer->m_KillAssistT.Set(RequestType::Global, results->GetInt(13));
		pPlayer->m_KillAssistCT.Set(RequestType::Global, results->GetInt(14));
	}
}

void CMysql::UpdateUser(CRankPlayer *pPlayer)
{
	if (!g_pConnection)
		return;

	pPlayer->PrintDebug(RequestType::Global);
	pPlayer->PrintDebug(RequestType::Map);
	pPlayer->PrintDebug(RequestType::Session);

	const char *name = g_pEngine->GetClientConVarValue(pPlayer->GetPlayerSlot(), "name");
	uint64 steamId64 = pPlayer->GetSteamId64();

	char szQuery[MAX_QUERY_SIZES];
	V_snprintf(szQuery, sizeof(szQuery), UPDATE_USER,
			   g_pConnection->Escape(name).c_str(), pPlayer->IsIgnoringAnnouce(),
			   pPlayer->m_Points.Get(RequestType::Map), pPlayer->m_DeathSuicide.Get(RequestType::Map),
			   pPlayer->m_DeathT.Get(RequestType::Map), pPlayer->m_DeathCT.Get(RequestType::Map),
			   pPlayer->m_BombPlanted.Get(RequestType::Map), pPlayer->m_BombExploded.Get(RequestType::Map),
			   pPlayer->m_BombDefused.Get(RequestType::Map), pPlayer->m_KillKnife.Get(RequestType::Map),
			   pPlayer->m_KillHeadshot.Get(RequestType::Map), pPlayer->m_KillT.Get(RequestType::Map),
			   pPlayer->m_KillCT.Get(RequestType::Map), pPlayer->m_TeamKillT.Get(RequestType::Map),
			   pPlayer->m_TeamKillCT.Get(RequestType::Map),
			   std::time(0), steamId64, this->EscapeRankReference().c_str(), this->g_iMapId);

	Debug("UpdateUser Request : %s", szQuery);

	g_pConnection->Query(szQuery, [](IMySQLQuery *cb) {});
}

void CMysql::RemoveFromOtherMap(CRankPlayer *pPlayer)
{
	if (!g_pConnection)
		return;

	// pPlayer->PrintDebug(false);
	// pPlayer->PrintDebug(true);

	const char *name = g_pEngine->GetClientConVarValue(pPlayer->GetPlayerSlot(), "name");
	uint64 steamId64 = pPlayer->GetSteamId64();

	char szQuery[MAX_QUERY_SIZES];
	V_snprintf(szQuery, sizeof(szQuery), REMOVE_USER_FROM_OTHERMAPS, steamId64, this->EscapeRankReference().c_str(), this->g_iMapId);

	Debug("UpdateUser Request : %s", szQuery);

	g_pConnection->Query(szQuery, [](IMySQLQuery *cb) {});
}

void CMysql::GetTopPlayers(bool global, std::function<void(std::map<std::string, int>)> callback)
{
	if (!g_pConnection)
		return;

	char szQuery[MAX_QUERY_SIZES];

	if (global)
		V_snprintf(szQuery, sizeof(szQuery), TOP, g_CConfig->GetMinimumPoints(), this->EscapeRankReference().c_str());
	else
		V_snprintf(szQuery, sizeof(szQuery), TOP_MAP, g_CConfig->GetMinimumPoints(), this->EscapeRankReference().c_str(), this->g_iMapId);

	g_pConnection->Query(szQuery, [callback, this](IMySQLQuery *cb)
						 { this->Query_TopPlayers(cb, callback); });
}

void CMysql::Query_TopPlayers(IMySQLQuery *cb, std::function<void(std::map<std::string, int>)> callback)
{
	std::map<std::string, int> players;

	IMySQLResult *results = cb->GetResultSet();
	while (results->FetchRow())
	{
		const char *name = results->GetString(0);
		int points = results->GetInt(1);
		players[name] = points;
	}

	callback(players);
}

void CMysql::GetRank(bool global, CRankPlayer *pPlayer, std::function<void(int)> callback)
{
	if (!g_pConnection)
		return;

	char szQuery[MAX_QUERY_SIZES];

	if (global)
		V_snprintf(szQuery, sizeof(szQuery), RANK, this->EscapeRankReference().c_str(), pPlayer->m_Points.Get(RequestType::Global));
	else
		V_snprintf(szQuery, sizeof(szQuery), RANK_MAP, pPlayer->m_Points.Get(RequestType::Map), this->EscapeRankReference().c_str(), this->g_iMapId);

	Debug(szQuery);

	g_pConnection->Query(szQuery, [callback, this](IMySQLQuery *cb)
						 { this->Query_Rank(cb, callback); });
}

void CMysql::Query_Rank(IMySQLQuery *cb, std::function<void(int)> callback)
{
	IMySQLResult *results = cb->GetResultSet();

	if (!results)
	{
		callback(-1);
		return;
	}

	if (!results->FetchRow())
	{
		callback(-1);
		return;
	}

	callback(results->GetInt(0) + 1);
}

std::string CMysql::EscapeRankReference()
{
	return g_pConnection->Escape(g_CConfig->GetRankReference());
}