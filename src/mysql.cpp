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
			Debug("Connected to database with server reference %s", this->EscapeRankReference().c_str());
			this->CreateDatabaseIfNotExist();
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

	V_snprintf(szQuery, sizeof(szQuery), SELECT_MAP, this->EscapeString(g_pGlobals->mapname.ToCStr()).c_str());

	Debug("Select map:  %s", szQuery);
	g_pConnection->Query(szQuery, [this](IMySQLQuery *cb)
						 { this->Query_GetMapId(cb); });
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
		Fatal("Invalid results for Query_GetMapId");
		return;
	}

	if (results->FetchRow())
	{
		this->g_iMapId = results->GetInt(0);
		return;
	}

	char szQuery[MAX_QUERY_SIZES];
	V_snprintf(szQuery, sizeof(szQuery), INSERT_MAP, this->EscapeString(g_pGlobals->mapname.ToCStr()).c_str());

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

	Debug("GetUserMap Request : %s", szQuery);

	g_pConnection->Query(szQuery, [pPlayer, this](IMySQLQuery *cb)
						 { this->Query_GetUserMap(cb, pPlayer); });

	V_snprintf(szQuery, sizeof(szQuery), SELECT_USER_GLOBAL, steamId64, this->EscapeRankReference().c_str(), this->g_iMapId);

	Debug("GetUserGlobal Request : %s", szQuery);

	g_pConnection->Query(szQuery, [pPlayer, this](IMySQLQuery *cb)
						 { this->Query_GetUserGlobal(cb, pPlayer); });
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
		V_snprintf(szQuery, sizeof(szQuery), INSERT_USER, steamId64, this->EscapeString(name).c_str(), this->EscapeRankReference().c_str(), this->g_iMapId);

		Debug("InsertUser request : %s", szQuery);

		g_pConnection->Query(szQuery, [pPlayer](IMySQLQuery *cb)
							 { 
								Debug("AUTH CALLBACK"); 
								pPlayer->SetDatabaseAuthenticated(); });
	}
	else
	{
		pPlayer->SetDatabaseAuthenticated();

		Debug("Connected points %i", results->GetInt(1));

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
			   this->EscapeString(name).c_str(), pPlayer->IsIgnoringAnnouce(),
			   pPlayer->m_Points.Get(RequestType::Global), pPlayer->m_DeathSuicide.Get(RequestType::Global),
			   pPlayer->m_DeathT.Get(RequestType::Global), pPlayer->m_DeathCT.Get(RequestType::Global),
			   pPlayer->m_BombPlanted.Get(RequestType::Global), pPlayer->m_BombExploded.Get(RequestType::Global),
			   pPlayer->m_BombDefused.Get(RequestType::Global), pPlayer->m_KillKnife.Get(RequestType::Global),
			   pPlayer->m_KillHeadshot.Get(RequestType::Global), pPlayer->m_KillT.Get(RequestType::Global),
			   pPlayer->m_KillCT.Get(RequestType::Global), pPlayer->m_TeamKillT.Get(RequestType::Global),
			   pPlayer->m_TeamKillCT.Get(RequestType::Global),
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
		V_snprintf(szQuery, sizeof(szQuery), TOP, this->EscapeRankReference().c_str(), g_CConfig->GetMinimumPoints());
	else
		V_snprintf(szQuery, sizeof(szQuery), TOP_MAP, g_CConfig->GetMinimumPoints(), this->EscapeRankReference().c_str());

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
		V_snprintf(szQuery, sizeof(szQuery), RANK_MAP, pPlayer->m_Points.Get(RequestType::Map), this->EscapeRankReference().c_str());

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
	return this->EscapeString(g_CConfig->GetRankReference());
}

std::string CMysql::EscapeString(const char *input)
{
	// return g_pConnection->Escape(input); // TODO: re-enable it after mysqm_mm escape function fix
	return this->SafeEscapeString(input);
}

std::string CMysql::SafeEscapeString(const char *input)
{
	std::size_t length = strlen(input);
	std::string escapedString;

	for (std::size_t i = 0; i < length; ++i)
	{
		switch (input[i])
		{
		case '\'':
			escapedString += "";
			break;
		case '"':
			escapedString += "\\\"";
			break;
		case '\\':
			escapedString += "\\\\";
			break;
		case '\0':
			escapedString += "\\0";
			break;
		case '\b':
			escapedString += "\\b";
			break;
		case '\n':
			escapedString += "\\n";
			break;
		case '\r':
			escapedString += "\\r";
			break;
		case '\t':
			escapedString += "\\t";
			break;
		case '\x1A':
			escapedString += "\\Z";
			break;
		case '%':
			escapedString += "\\%";
			break;
		case '_':
			escapedString += "\\_";
			break;
		default:
			escapedString += input[i];
		}
	}

	return escapedString;
}