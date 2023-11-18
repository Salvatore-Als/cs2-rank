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

	Debug("Create Database request 1: %s", CREATE_USERS_TABLE);
	Debug("Create Database request 2: %s", CREATE_REFERENCES_TABLE);

	char szQuery[MAX_QUERY_SIZES];
	V_snprintf(szQuery, sizeof(szQuery), SELECT_REFERENCE, this->EscapeRankReference().c_str());

	g_pConnection->Query(szQuery, [this](IMySQLQuery *cb)
						 { this->Query_GetRankReference(cb); });
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

void CMysql::GetUser(CRankPlayer *pPlayer)
{
	if (!g_pConnection)
		return;

	uint64 steamId64 = pPlayer->GetSteamId64();

	char szQuery[MAX_QUERY_SIZES];
	V_snprintf(szQuery, sizeof(szQuery), SELECT_USER, steamId64, this->EscapeRankReference().c_str());

	Debug("GetUser Request : %s", szQuery);

	g_pConnection->Query(szQuery, [pPlayer, this](IMySQLQuery *cb)
						 { this->Query_GetUser(cb, pPlayer); });
}

void CMysql::Query_GetUser(IMySQLQuery *cb, CRankPlayer *pPlayer)
{
	IMySQLResult *results = cb->GetResultSet();

	if (!results)
	{
		Fatal("Invalid results for Query_GetUser");
		return;
	}

	if (!results->FetchRow())
	{
		pPlayer->InitStats(true);

		const char *name = g_pEngine->GetClientConVarValue(pPlayer->GetPlayerSlot(), "name");
		uint64 steamId64 = pPlayer->GetSteamId64();

		// NOTE : Only set the authid and name, because the other data have a default value set on the database schema

		char szQuery[MAX_QUERY_SIZES];
		V_snprintf(szQuery, sizeof(szQuery), INSERT_USER, steamId64, this->EscapeString(name).c_str(), this->EscapeRankReference().c_str());

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
		pPlayer->SetPoints(results->GetInt(1), false);
		pPlayer->SetDeathSuicide(results->GetInt(2), false);
		pPlayer->SetDeathT(results->GetInt(3), false);
		pPlayer->SetDeathCT(results->GetInt(4), false);
		pPlayer->SetBombPlanted(results->GetInt(5), false);
		pPlayer->SetBombExploded(results->GetInt(6), false);
		pPlayer->SetBombDefused(results->GetInt(7), false);
		pPlayer->SetKillKnife(results->GetInt(8), false);
		pPlayer->SetKillHeadshot(results->GetInt(9)), false;
		pPlayer->SetKillT(results->GetInt(10), false);
		pPlayer->SetKillCT(results->GetInt(11), false);
		pPlayer->SetTeamKillT(results->GetInt(12), false);
		pPlayer->SetTeamKillCT(results->GetInt(13), false);
		pPlayer->SetKillAssistT(results->GetInt(14), false);
		pPlayer->SetKillAssistCT(results->GetInt(15), false);
	}
}

void CMysql::UpdateUser(CRankPlayer *pPlayer)
{
	if (!g_pConnection)
		return;

	pPlayer->PrintDebug(false);
	pPlayer->PrintDebug(true);

	const char *name = g_pEngine->GetClientConVarValue(pPlayer->GetPlayerSlot(), "name");
	uint64 steamId64 = pPlayer->GetSteamId64();

	char szQuery[MAX_QUERY_SIZES];
	V_snprintf(szQuery, sizeof(szQuery), UPDATE_USER, this->EscapeString(name).c_str(), pPlayer->IsIgnoringAnnouce(), pPlayer->GetPoints(), pPlayer->GetDeathSuicide(), pPlayer->GetDeathT(), pPlayer->GetDeathCT(), pPlayer->GetBombPlanted(), pPlayer->GetBombExploded(), pPlayer->GetBombDefused(), pPlayer->GetKillKnife(), pPlayer->GetKillHeadshot(), pPlayer->GetKillT(), pPlayer->GetKillCT(), pPlayer->GetTeamKillT(), pPlayer->GetKillCT(), std::time(0), steamId64, this->EscapeRankReference().c_str());

	Debug("UpdateUser Request : %s", szQuery);

	g_pConnection->Query(szQuery, [](IMySQLQuery *cb) {});
}

void CMysql::GetTopPlayers(std::function<void(std::map<std::string, int>)> callback)
{
	if (!g_pConnection)
		return;

	char szQuery[MAX_QUERY_SIZES];
	V_snprintf(szQuery, sizeof(szQuery), TOP, g_CConfig->GetMinimumPoints(), this->EscapeRankReference().c_str());

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

void CMysql::GetRank(CRankPlayer *pPlayer, std::function<void(int)> callback)
{
	if (!g_pConnection)
		return;

	char szQuery[MAX_QUERY_SIZES];
	V_snprintf(szQuery, sizeof(szQuery), RANK, pPlayer->GetPoints(false), this->EscapeRankReference().c_str());

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