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
#include "basecommands.h"
#include "config.h"

#define VPROF_ENABLED
#include "tier0/vprof.h"

size_t UTIL_Format(char *buffer, size_t maxlength, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	size_t len = vsnprintf(buffer, maxlength, fmt, ap);
	va_end(ap);

	if (len >= maxlength)
	{
		len = maxlength - 1;
		buffer[len] = '\0';
	}

	return len;
}

void Debug(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[1024] = {};
	V_vsnprintf(buf, sizeof(buf) - 1, msg, args);

	ConColorMsg(Color(255, 0, 255, 255), DEBUG_PREFIX "%s\n", buf);

	va_end(args);
}

void Fatal(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[1024] = {};
	V_vsnprintf(buf, sizeof(buf) - 1, msg, args);

	ConColorMsg(Color(255, 0, 0, 255), DEBUG_PREFIX "%s\n", buf);

	va_end(args);
}

void Warn(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[1024] = {};
	V_vsnprintf(buf, sizeof(buf) - 1, msg, args);

	ConColorMsg(Color(255, 165, 0, 255), DEBUG_PREFIX "%s\n", buf);

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
CConfig *g_CConfig = nullptr;

CEntitySystem *g_pEntitySystem = nullptr;
CGameResourceService *g_gGameResourceServiceServe = nullptr;
CGameConfig *g_CGameConfig;
CSchemaSystem *g_pSchemaSystem2 = nullptr;

float g_flUniversalTime;
float g_flLastTickedTime;
bool g_bHasTicked;
PluginId g_pluginId;

CUtlMap<uint32, CChatCommand *> g_Commands(0, 0, DefLessFunc(uint32));

CGlobalVars *GetServerGlobals()
{
	return g_pEngine->GetServerGlobals();
}

PLUGIN_EXPOSE(CPlugin, g_CPlugin);
bool CPlugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	g_pluginId = id;

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

	g_CConfig = new CConfig();
	char szConfigError[255] = "";
	if (!g_CConfig->Init(szConfigError, sizeof(szConfigError)))
	{
		snprintf(error, maxlen, "Unable to init the configuration: %s", szConfigError);
		return false;
	}

	int ret;
	g_pMysqlClient = (IMySQLClient *)g_SMAPI->MetaFactory(MYSQLMM_INTERFACE, &ret, NULL);

	if (ret == META_IFACE_FAILED)
	{
		snprintf(error, maxlen, "Missing MYSQL plugin");
		return false;
	}

	g_CGameConfig = new CGameConfig();
	char szGameConfigError[255] = "";
	if (!g_CGameConfig->Init(g_pFullFileSystem, szGameConfigError, sizeof(szGameConfigError)))
	{
		snprintf(error, maxlen, "Could not read %s: %s", g_CGameConfig->GetPath().c_str(), szGameConfigError);
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
		return false;
	}

	CModule *pModuleEngine = g_CAddresses->GetModuleEngine();
	g_gGameResourceServiceServe = (CGameResourceService *)pModuleEngine->FindInterface(GAMERESOURCESERVICESERVER_INTERFACE_VERSION);
	if (!g_gGameResourceServiceServe)
	{
		snprintf(error, maxlen, "Unable to find the CGameResourceService interface");
		return false;
	}

	g_pGameEventManager = (IGameEventManager2 *)(CALL_VIRTUAL(uintptr_t, 91, g_pSource2Server) - 8);
	if (!g_pGameEventManager)
	{
		snprintf(error, maxlen, "Unable to find the IGameEventManager2");
		return false;
	}

	if (!InitDetours(g_CGameConfig))
	{
		snprintf(error, maxlen, "Unable to init detours");
		return false;
	}

	g_pEntitySystem = g_gGameResourceServiceServe->GetGameEntitySystem();

	g_CChat = new CChat();
	g_CPlayerManager = new CPlayerManager();
	g_CMysql = new CMysql();

	g_pCVar = icvar;
	ConVar_Register(FCVAR_RELEASE | FCVAR_CLIENT_CAN_EXECUTE | FCVAR_GAMEDLL);

	g_pEngine->ServerCommand("sv_hibernate_when_empty 0");

	srand(time(0));
	new CTimer(1.0f, true, []()
			   {	g_CPlayerManager->TryAuthenticate(); return 1.0f; });

	return true;
}

void CPlugin::ForceUnload()
{
	char buffer[256];
	UTIL_Format(buffer, sizeof(buffer), "meta unload %i", g_pluginId);

	g_pEngine->ServerCommand(buffer);
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

	if (g_CGameConfig)
		delete g_CGameConfig;

	if (g_CChat)
		delete g_CChat;

	if (g_CConfig)
	{
		g_CConfig->Destroy();
		delete g_CConfig;
	}
	if (g_CMysql)
	{
		g_CMysql->Destroy();
		delete g_CMysql;
	}

	UnregisterEventListeners();
	FlushAllDetours();
	RemoveTimers();

	g_Commands.Purge();

	return true;
}

void CPlugin::Hook_StartupServer(const GameSessionConfiguration_t &config, ISource2WorldSession *, const char *)
{
	g_pGlobals = GetServerGlobals();

	if (g_pGlobals == nullptr)
		Fatal("Failed to lookup g_pGlobals\n");

	g_pEntitySystem = g_gGameResourceServiceServe->GetGameEntitySystem();

	if (g_bHasTicked)
		RemoveMapTimers();

	g_bHasTicked = false;

	RegisterEventListeners();
}

void CPlugin::Hook_GameFrame(bool simulating, bool bFirstTick, bool bLastTick)
{
	VPROF_ENTER_SCOPE(__FUNCTION__);

	if (simulating && g_bHasTicked)
		g_flUniversalTime += g_pGlobals->curtime - g_flLastTickedTime;
	else
		g_flUniversalTime += g_pGlobals->interval_per_tick;

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
		g_CPlayerManager->OnBotConnected(slot);
}

bool CPlugin::Hook_ClientConnect(CPlayerSlot slot, const char *pszName, uint64 xuid, const char *pszNetworkID, bool unk1, CBufferString *pRejectReason)
{
	if (!g_CPlayerManager->OnClientConnected(slot))
		RETURN_META_VALUE(MRES_SUPERCEDE, false);

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

	Debug("- GetPoints S %i", pPlayer->GetPoints(true));
	Debug("- GetDeathSuicide S %i", pPlayer->GetDeathSuicide(true));
	Debug("- GetDeathT S %i", pPlayer->GetDeathT(true));
	Debug("- GetDeathCT S %i", pPlayer->GetDeathCT(true));
	Debug("- GetBombPlanted S %i", pPlayer->GetBombPlanted(true));
	Debug("- GetBombExploded S %i", pPlayer->GetBombExploded(true));
	Debug("- GetBombDefused S %i", pPlayer->GetBombDefused(true));
	Debug("- GetKillKnife S %i", pPlayer->GetKillKnife(true));
	Debug("- GetKillHeadshotS  %i", pPlayer->GetKillHeadshot(true));
	Debug("- GetKillT S %i", pPlayer->GetKillT(true));
	Debug("- GetKillCT S %i", pPlayer->GetKillCT(true));
	Debug("- GetTeamKillT S %i", pPlayer->GetTeamKillT(true));
	Debug("- GetTeamKillCT S %i", pPlayer->GetTeamKillCT(true));
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
	Debug("- GetPoints %i", pPlayer->GetPoints(false));
	Debug("- GetDeathSuicide %i", pPlayer->GetDeathSuicide(false));
	Debug("- GetDeathT %i", pPlayer->GetDeathT(false));
	Debug("- GetDeathCT %i", pPlayer->GetDeathCT(false));
	Debug("- GetBombPlanted %i", pPlayer->GetBombPlanted(false));
	Debug("- GetBombExploded %i", pPlayer->GetBombExploded(false));
	Debug("- GetBombDefused %i", pPlayer->GetBombDefused(false));
	Debug("- GetKillKnife %i", pPlayer->GetKillKnife(false));
	Debug("- GetKillHeadshot %i", pPlayer->GetKillHeadshot(false));
	Debug("- GetKillT %i", pPlayer->GetKillT(false));
	Debug("- GetKillCT %i", pPlayer->GetKillCT(false));
	Debug("- GetTeamKillT %i", pPlayer->GetTeamKillT(false));
	Debug("- GetTeamKillCT %i", pPlayer->GetTeamKillCT(false));

	pPlayer->AddPoints(10);
	pPlayer->AddDeathSuicide(10);
	pPlayer->AddDeathT(10);
	pPlayer->AddDeathCT(10);
	pPlayer->AddBombPlanted(10);
	pPlayer->AddBombExploded(10);
	pPlayer->AddBombDefused(10);
	pPlayer->AddKillKnife(10);
	pPlayer->AddKillHeadshot(10);
	pPlayer->AddKillT(10);
	pPlayer->AddKillCT(10);
	pPlayer->AddTeamKillT(10);
	pPlayer->AddTeamKillCT(10);

	Debug("After SET");
	Debug("- GetPoints %i", pPlayer->GetPoints(false));
	Debug("- GetDeathSuicide %i", pPlayer->GetDeathSuicide(false));
	Debug("- GetDeathT %i", pPlayer->GetDeathT(false));
	Debug("- GetDeathCT %i", pPlayer->GetDeathCT(false));
	Debug("- GetBombPlanted %i", pPlayer->GetBombPlanted(false));
	Debug("- GetBombExploded %i", pPlayer->GetBombExploded(false));
	Debug("- GetBombDefused %i", pPlayer->GetBombDefused(false));
	Debug("- GetKillKnife %i", pPlayer->GetKillKnife(false));
	Debug("- GetKillHeadshot %i", pPlayer->GetKillHeadshot(false));
	Debug("- GetKillT %i", pPlayer->GetKillT(false));
	Debug("- GetKillCT %i", pPlayer->GetKillCT(false));
	Debug("- GetTeamKillT %i", pPlayer->GetTeamKillT(false));
	Debug("- GetTeamKillCT %i", pPlayer->GetTeamKillCT(false));

	Debug("Saving in database");
	pPlayer->SaveOnDatabase();
}