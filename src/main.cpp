#include <stdio.h>
#include "main.h"
#include "gameconfig.h"
#include <string>
#include "abstract.h"
#include "addresses.h"
#include "utils/module.h"
#include "entity2/entitysystem.h"
#include "entity/cbaseentity.h"
#include "entity/ccsplayercontroller.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "detours.h"
#include "cs2_sdk/interfaces/cgameresourceserviceserver.h"
#include "chat.h"
#include "eventlistener.h"
#include "mysql.h"
#include "player.h"
#include <vendor/mysql/include/mysql_mm.h>
#include "ctimer.h"

#define VPROF_ENABLED
#include "tier0/vprof.h"

void Debug(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[1024] = {};
	V_vsnprintf(buf, sizeof(buf) - 1, msg, args);

	ConColorMsg(Color(255, 0, 255, 255), "%s %s\n", PREFIX, buf);

	va_end(args);
}

void Fatal(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[1024] = {};
	V_vsnprintf(buf, sizeof(buf) - 1, msg, args);

	ConColorMsg(Color(255, 0, 0, 255), "%s %s\n", PREFIX, buf);

	va_end(args);
}

void Warn(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[1024] = {};
	V_vsnprintf(buf, sizeof(buf) - 1, msg, args);

	ConColorMsg(Color(255, 165, 0, 255), "%s %s\n", PREFIX, buf);

	va_end(args);
}

class GameSessionConfiguration_t
{
};

SH_DECL_HOOK3_void(INetworkServerService, StartupServer, SH_NOATTRIB, 0, const GameSessionConfiguration_t &, ISource2WorldSession *, const char *);
SH_DECL_HOOK3_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool, bool, bool);
SH_DECL_HOOK5_void(IServerGameClients, ClientDisconnect, SH_NOATTRIB, 0, CPlayerSlot, int, const char *, uint64, const char *);
SH_DECL_HOOK6_void(IServerGameClients, OnClientConnected, SH_NOATTRIB, 0, CPlayerSlot, const char *, uint64, const char *, const char *, bool);
SH_DECL_HOOK6(IServerGameClients, ClientConnect, SH_NOATTRIB, 0, bool, CPlayerSlot, const char *, uint64, const char *, bool, CBufferString *);

CPlugin g_CPlugin;
// IServerGameDLL *g_pSource2Server = nullptr;
ISource2GameClients *g_pGameclients = nullptr;
IVEngineServer2 *g_pEngine = nullptr;
ICvar *icvar = nullptr;
IGameEventManager2 *g_pGameEventManager = nullptr;

IMySQLClient *g_pMysqlClient;
IMySQLConnection *g_pConnection;
CGlobalVars *g_pGlobals = nullptr;
CAddresses *g_CAddresses = nullptr;
CChat *g_CChat = nullptr;
CMysql *g_CMysql = nullptr;
CPlayerManager *g_CPlayerManager = nullptr;

CEntitySystem *g_pEntitySystem = nullptr;
CGameResourceService *g_gGameResourceServiceServe = nullptr;
CGameConfig *g_CGameConfig;
CSchemaSystem *g_pSchemaSystem2 = nullptr;

float g_flUniversalTime;
float g_flLastTickedTime;
bool g_bHasTicked;

CGlobalVars *GetServerGlobals()
{
	return g_pEngine->GetServerGlobals();
}

PLUGIN_EXPOSE(CPlugin, g_CPlugin);
bool CPlugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_ANY(GetServerFactory, g_pGameclients, ISource2GameClients, SOURCE2GAMECLIENTS_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pEngine, IVEngineServer2, SOURCE2ENGINETOSERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetFileSystemFactory, g_pFullFileSystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, g_pSource2GameClients, IServerGameClients, SOURCE2GAMECLIENTS_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, g_pSource2Server, ISource2Server, SOURCE2SERVER_INTERFACE_VERSION);

	SH_ADD_HOOK_MEMFUNC(INetworkServerService, StartupServer, g_pNetworkServerService, this, &CPlugin::Hook_StartupServer, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, OnClientConnected, g_pSource2GameClients, this, &CPlugin::Hook_OnClientConnected, false);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientConnect, g_pSource2GameClients, this, &CPlugin::Hook_ClientConnect, false);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, g_pSource2GameClients, this, &CPlugin::Hook_ClientDisconnect, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, GameFrame, g_pSource2Server, this, &CPlugin::Hook_GameFrame, true);

	int ret;
	g_pMysqlClient = (IMySQLClient *)g_SMAPI->MetaFactory(MYSQLMM_INTERFACE, &ret, NULL);

	if (ret == META_IFACE_FAILED)
	{
		snprintf(error, maxlen, "Missing MYSQL plugin");
		Fatal(error);
		return false;
	}

	g_CGameConfig = new CGameConfig();
	char szGameConfigError[255] = "";
	if (!g_CGameConfig->Init(g_pFullFileSystem, szGameConfigError, sizeof(szGameConfigError)))
	{
		snprintf(error, maxlen, "Could not read %s: %s", g_CGameConfig->GetPath().c_str(), szGameConfigError);
		Fatal(error);
		return false;
	}

	g_pGlobals = GetServerGlobals();

	g_CAddresses = new CAddresses();
	g_CAddresses->Init(g_CGameConfig);

	CModule *pSchemaSystem = g_CAddresses->GetShemaSystem();
	g_pSchemaSystem2 = (CSchemaSystem *)pSchemaSystem->FindInterface(SCHEMASYSTEM_INTERFACE_VERSION);
	if (!g_pSchemaSystem2)
	{
		snprintf(error, maxlen, "Unable to find CSchemaSystem interface");
		Fatal(error);
		return false;
	}

	CModule *pModuleEngine = g_CAddresses->GetModuleEngine();
	g_gGameResourceServiceServe = (CGameResourceService *)pModuleEngine->FindInterface(GAMERESOURCESERVICESERVER_INTERFACE_VERSION);
	if (!g_gGameResourceServiceServe)
	{
		snprintf(error, maxlen, "Unable to find the CGameResourceService interface");
		Fatal(error);
		return false;
	}

	g_pGameEventManager = (IGameEventManager2 *)(CALL_VIRTUAL(uintptr_t, 91, g_pSource2Server) - 8);
	if (!g_pGameEventManager)
	{
		snprintf(error, maxlen, "Unable to find the IGameEventManager2");
		Fatal(error);
		return false;
	}

	if (!InitDetours(g_CGameConfig))
	{
		snprintf(error, maxlen, "Unable to init detours");
		Fatal(error);
		return false;
	}

	g_pEntitySystem = g_gGameResourceServiceServe->GetGameEntitySystem();

	g_CMysql = new CMysql();
	g_CChat = new CChat();

	g_CPlayerManager = new CPlayerManager();

	new CTimer(1.0f, true, []()
			   { g_CPlayerManager->TryAuthenticate(); return 1.0f; });

	g_pCVar = icvar;
	ConVar_Register(FCVAR_RELEASE | FCVAR_CLIENT_CAN_EXECUTE | FCVAR_GAMEDLL);

	srand(time(0));

	return true;
}

bool CPlugin::Unload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK_MEMFUNC(INetworkServerService, StartupServer, g_pNetworkServerService, this, &CPlugin::Hook_StartupServer, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, OnClientConnected, g_pSource2GameClients, this, &CPlugin::Hook_OnClientConnected, false);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientConnect, g_pSource2GameClients, this, &CPlugin::Hook_ClientConnect, false);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, g_pSource2GameClients, this, &CPlugin::Hook_ClientDisconnect, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, GameFrame, g_pSource2Server, this, &CPlugin::Hook_GameFrame, true);

	if (g_CAddresses)
	{
		g_CAddresses->Clean();
		delete g_CAddresses;
	}

	delete g_CGameConfig;
	delete g_CChat;

	if (g_CMysql)
	{
		g_CMysql->Destroy();
		delete g_CMysql;
	}

	UnregisterEventListeners();
	FlushAllDetours();
	RemoveTimers();

	return true;
}

void CPlugin::Hook_StartupServer(const GameSessionConfiguration_t &config, ISource2WorldSession *, const char *)
{
	g_pGlobals = GetServerGlobals();

	if (g_pGlobals == nullptr)
	{
		Fatal("Failed to lookup g_pGlobals\n");
	}

	g_pEntitySystem = g_gGameResourceServiceServe->GetGameEntitySystem();

	if (g_bHasTicked)
	{
		RemoveMapTimers();
	}

	g_bHasTicked = false;

	RegisterEventListeners();
}

void CPlugin::Hook_GameFrame(bool simulating, bool bFirstTick, bool bLastTick)
{
	VPROF_ENTER_SCOPE(__FUNCTION__);

	if (simulating && g_bHasTicked)
	{
		g_flUniversalTime += g_pGlobals->curtime - g_flLastTickedTime;
	}
	else
	{
		g_flUniversalTime += g_pGlobals->interval_per_tick;
	}

	g_flLastTickedTime = g_pGlobals->curtime;
	g_bHasTicked = true;

	for (int i = g_timers.Tail(); i != g_timers.InvalidIndex();)
	{
		auto timer = g_timers[i];

		int prevIndex = i;
		i = g_timers.Previous(i);

		if (timer->m_flLastExecute == -1)
			timer->m_flLastExecute = g_flUniversalTime;

		// Timer execute
		if (timer->m_flLastExecute + timer->m_flInterval <= g_flUniversalTime)
		{
			if (!timer->Execute())
			{
				delete timer;
				g_timers.Remove(prevIndex);
			}
			else
			{
				timer->m_flLastExecute = g_flUniversalTime;
			}
		}
	}

	VPROF_EXIT_SCOPE();
}

void CPlugin::Hook_OnClientConnected(CPlayerSlot slot, const char *pszName, uint64 xuid, const char *pszNetworkID, const char *pszAddress, bool bFakePlayer)
{
	if (bFakePlayer)
	{
		g_CPlayerManager->OnBotConnected(slot);
	}
}

bool CPlugin::Hook_ClientConnect(CPlayerSlot slot, const char *pszName, uint64 xuid, const char *pszNetworkID, bool unk1, CBufferString *pRejectReason)
{
	if (!g_CPlayerManager->OnClientConnected(slot))
	{
		RETURN_META_VALUE(MRES_SUPERCEDE, false);
	}

	RETURN_META_VALUE(MRES_IGNORED, true);
}

void CPlugin::Hook_ClientDisconnect(CPlayerSlot slot, int reason, const char *pszName, uint64 xuid, const char *pszNetworkID)
{
	g_CPlayerManager->OnClientDisconnect(slot);
}

void CPlugin::AllPluginsLoaded()
{
}

bool CPlugin::Pause(char *error, size_t maxlen)
{
	return true;
}

bool CPlugin::Unpause(char *error, size_t maxlen)
{
	return true;
}

const char *CPlugin::GetLicense()
{
	return "Public Domain";
}

const char *CPlugin::GetVersion()
{
	return "a1.0";
}

const char *CPlugin::GetDate()
{
	return __DATE__;
}

const char *CPlugin::GetLogTag()
{
	return "STUB";
}

const char *CPlugin::GetAuthor()
{
	return "Kriax, VeryGames";
}

const char *CPlugin::GetDescription()
{
	return PLUGIN_DESCRIPTION;
}

const char *CPlugin::GetName()
{
	return PLUGIN_NAME;
}

const char *CPlugin::GetURL()
{
	return "https://www.verygames.net";
}



CON_COMMAND_EXTERN(rank_debugprint, Command_DebugPrint, "");
void Command_DebugPrint(const CCommandContext &context, const CCommand &args)
{
	Debug("Rank Debug");

	CPlayerSlot slot = context.GetPlayerSlot();

	if (slot.Get() < 0)
	{
		Debug("Invalid player slot %d", slot.Get());
		return;
	}

	CRankPlayer *pPlayer = g_CPlayerManager->GetPlayer(slot);
	if (!pPlayer)
	{
		Debug("Player is invalid");
		return;
	}

	if (!pPlayer->IsAuthenticated())
	{
		Debug("Player is not authenticated on steam");
		return;
	}

	if (!pPlayer->IsDatabaseAuthenticated())
	{
		Debug("Player is not authenticated on database");
		return;
	}

	Debug("- GetPoints %i", pPlayer->GetPoints());
	Debug("- GetDeathSuicide %i", pPlayer->GetDeathSuicide());
	Debug("- GetDeathT %i", pPlayer->GetDeathT());
	Debug("- GetDeathCT %i", pPlayer->GetDeathCT());
	Debug("- GetBombPlanted %i", pPlayer->GetBombPlanted());
	Debug("- GetBombExploded %i", pPlayer->GetBombExploded());
	Debug("- GetBombDefused %i", pPlayer->GetBombDefused());
	Debug("- GetKillKnife %i", pPlayer->GetKillKnife());
	Debug("- GetKillHeadshot %i", pPlayer->GetKillHeadshot());
	Debug("- GetKillT %i", pPlayer->GetKillT());
	Debug("- GetKillCT %i", pPlayer->GetKillCT());
	Debug("- GetTeamKillT %i", pPlayer->GetTeamKillT());
	Debug("- GetTeamKillCT %i", pPlayer->GetTeamKillCT());
}

CON_COMMAND_EXTERN(rank_debugadd, Command_DebugAdd, "");
void Command_DebugAdd(const CCommandContext &context, const CCommand &args)
{
	Debug("Rank Debug");

	CPlayerSlot slot = context.GetPlayerSlot();

	if (slot.Get() < 0)
	{
		Debug("Invalid player slot", slot.Get());
		return;
	}

	Debug("Should add a value for each data and update the player");

	CRankPlayer *pPlayer = g_CPlayerManager->GetPlayer(slot);
	if (!pPlayer)
	{
		Debug("Player is invalid");
		return;
	}

	if (!pPlayer->IsAuthenticated())
	{
		Debug("Player is not authenticated on steam");
		return;
	}

	if (!pPlayer->IsDatabaseAuthenticated())
	{
		Debug("Player is not authenticated on database");
		return;
	}

	Debug("Before SET");
	Debug("- GetPoints %i", pPlayer->GetPoints());
	Debug("- GetDeathSuicide %i", pPlayer->GetDeathSuicide());
	Debug("- GetDeathT %i", pPlayer->GetDeathT());
	Debug("- GetDeathCT %i", pPlayer->GetDeathCT());
	Debug("- GetBombPlanted %i", pPlayer->GetBombPlanted());
	Debug("- GetBombExploded %i", pPlayer->GetBombExploded());
	Debug("- GetBombDefused %i", pPlayer->GetBombDefused());
	Debug("- GetKillKnife %i", pPlayer->GetKillKnife());
	Debug("- GetKillHeadshot %i", pPlayer->GetKillHeadshot());
	Debug("- GetKillT %i", pPlayer->GetKillT());
	Debug("- GetKillCT %i", pPlayer->GetKillCT());
	Debug("- GetTeamKillT %i", pPlayer->GetTeamKillT());
	Debug("- GetTeamKillCT %i", pPlayer->GetTeamKillCT());

	pPlayer->SetPoints(pPlayer->GetPoints() + 10);
	pPlayer->SetDeathSuicide(pPlayer->GetDeathSuicide() + 10);
	pPlayer->SetDeathT(pPlayer->GetDeathT() + 10);
	pPlayer->SetDeathCT(pPlayer->GetDeathCT() + 10);
	pPlayer->SetBombPlanted(pPlayer->GetBombPlanted() + 10);
	pPlayer->SetBombExploded(pPlayer->GetBombExploded() + 10);
	pPlayer->SetBombDefused(pPlayer->GetBombDefused() + 10);
	pPlayer->SetKillKnife(pPlayer->GetKillKnife() + 10);
	pPlayer->SetKillHeadshot(pPlayer->GetKillHeadshot() + 10);
	pPlayer->SetKillT(pPlayer->GetKillT() + 10);
	pPlayer->SetKillCT(pPlayer->GetKillCT() + 10);
	pPlayer->SetTeamKillT(pPlayer->GetTeamKillT() + 10);
	pPlayer->SetTeamKillCT(pPlayer->GetTeamKillCT() + 10);

	Debug("After SET");
	Debug("- GetPoints %i", pPlayer->GetPoints());
	Debug("- GetDeathSuicide %i", pPlayer->GetDeathSuicide());
	Debug("- GetDeathT %i", pPlayer->GetDeathT());
	Debug("- GetDeathCT %i", pPlayer->GetDeathCT());
	Debug("- GetBombPlanted %i", pPlayer->GetBombPlanted());
	Debug("- GetBombExploded %i", pPlayer->GetBombExploded());
	Debug("- GetBombDefused %i", pPlayer->GetBombDefused());
	Debug("- GetKillKnife %i", pPlayer->GetKillKnife());
	Debug("- GetKillHeadshot %i", pPlayer->GetKillHeadshot());
	Debug("- GetKillT %i", pPlayer->GetKillT());
	Debug("- GetKillCT %i", pPlayer->GetKillCT());
	Debug("- GetTeamKillT %i", pPlayer->GetTeamKillT());
	Debug("- GetTeamKillCT %i", pPlayer->GetTeamKillCT());

	Debug("Saving in database");
	pPlayer->SaveOnDatabase();
}