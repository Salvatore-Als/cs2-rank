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
#include "interfaces/cschemasystem.h"

#define VPROF_ENABLED
#include "tier0/vprof.h"

size_t UTIL_Format(char *buffer, size_t maxlength, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	size_t len = V_vsnprintf(buffer, maxlength, fmt, ap);
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
	#ifdef _DEBUG
		va_list args;
		va_start(args, msg);

		char buf[8064] = {};
		V_vsnprintf(buf, sizeof(buf) - 1, msg, args);

		ConColorMsg(Color(255, 0, 255, 255), DEBUG_PREFIX "%s\n\n", buf);

		va_end(args);
	#endif
}

void Fatal(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[8064] = {};
	V_vsnprintf(buf, sizeof(buf) - 1, msg, args);

	ConColorMsg(Color(255, 0, 0, 255), DEBUG_PREFIX "%s\n\n", buf);

	va_end(args);
}

void Warn(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[8064] = {};
	V_vsnprintf(buf, sizeof(buf) - 1, msg, args);

	ConColorMsg(Color(255, 165, 0, 255), DEBUG_PREFIX "%s\n\n", buf);

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
SH_DECL_HOOK3_void(ICvar, DispatchConCommand, SH_NOATTRIB, 0, ConCommandHandle, const CCommandContext &, const CCommand &);

CPlugin g_CPlugin;
// IServerGameDLL *g_pSource2Server = nullptr;
ISource2GameClients *g_pGameclients = nullptr;
IVEngineServer2 *g_pEngine = nullptr;
ICvar *icvar = nullptr;
IGameEventManager2 *g_pGameEventManager = nullptr;
CSchemaSystem *g_pSchemaSystem2 = nullptr;

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

float g_flUniversalTime;
float g_flLastTickedTime;
bool g_bHasTicked;
PluginId g_pluginId;

CUtlMap<uint32, CChatCommand *> g_Commands(0, 0, DefLessFunc(uint32));

CGlobalVars *GetServerGlobals()
{
	return g_pEngine->GetServerGlobals();
}

/*CEntitySystem *GetEntitySystem()
{
	static int offset = g_CGameConfig->GetOffset("GameEntitySystem");
	return *reinterpret_cast<CGameEntitySystem **>((uintptr_t)(g_pGameResourceServiceServer) + offset);
}*/

PLUGIN_EXPOSE(CPlugin, g_CPlugin);
bool CPlugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	g_pluginId = id;

	PLUGIN_SAVEVARS();

	g_SMAPI->AddListener(this, this);

	GET_V_IFACE_ANY(GetServerFactory, g_pGameclients, ISource2GameClients, SOURCE2GAMECLIENTS_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pEngine, IVEngineServer2, SOURCE2ENGINETOSERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetFileSystemFactory, g_pFullFileSystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, g_pSource2GameClients, IServerGameClients, SOURCE2GAMECLIENTS_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, g_pSource2Server, ISource2Server, SOURCE2SERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pGameResourceServiceServer, IGameResourceServiceServer, GAMERESOURCESERVICESERVER_INTERFACE_VERSION);
	//GET_V_IFACE_CURRENT(GetEngineFactory, g_pSchemaSystem2, CSchemaSystem, SCHEMASYSTEM_INTERFACE_VERSION);

	g_pCVar = icvar;

	SH_ADD_HOOK_MEMFUNC(INetworkServerService, StartupServer, g_pNetworkServerService, this, &CPlugin::Hook_StartupServer, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, OnClientConnected, g_pSource2GameClients, this, &CPlugin::Hook_OnClientConnected, false);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientConnect, g_pSource2GameClients, this, &CPlugin::Hook_ClientConnect, false);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, g_pSource2GameClients, this, &CPlugin::Hook_ClientDisconnect, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, GameFrame, g_pSource2Server, this, &CPlugin::Hook_GameFrame, true);
	SH_ADD_HOOK_MEMFUNC(ICvar, DispatchConCommand, g_pCVar, this, &CPlugin::Hook_DispatchConCommand, false);

	g_CConfig = new CConfig();
	char szConfigError[255] = "";
	if (!g_CConfig->Init(szConfigError, sizeof(szConfigError)))
	{
		snprintf(error, maxlen, "Unable to init the configuration: %s", szConfigError);
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

	//g_pEntitySystem = GetEntitySystem();
	g_pEntitySystem = g_gGameResourceServiceServe->GetGameEntitySystem();

	g_CChat = new CChat();
	g_CPlayerManager = new CPlayerManager();

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
	SH_REMOVE_HOOK_MEMFUNC(ICvar, DispatchConCommand, g_pCVar, this, &CPlugin::Hook_DispatchConCommand, false);

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
	{
		Fatal("Failed to lookup g_pGlobals, unloading");
		this->ForceUnload();
	}

	//g_pEntitySystem = GetEntitySystem();
	g_pEntitySystem = g_gGameResourceServiceServe->GetGameEntitySystem();
	
	if (g_bHasTicked)
		RemoveMapTimers();

	g_bHasTicked = false;

	RegisterEventListeners();

	/*if (g_CMysql != nullptr && g_CMysql->IsConnected())
	{
		g_CMysql->Destroy();
		delete g_CMysql;
	}

	g_CMysql = new CMysql();*/

	if(g_CMysql && g_CMysql->IsConnected())
		g_CMysql->CreateDatabaseIfNotExist();
}

void CPlugin::Hook_GameFrame(bool simulating, bool bFirstTick, bool bLastTick)
{
	VPROF_ENTER_SCOPE(__FUNCTION__);

	if (g_pGlobals)
	{
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

void CPlugin::Hook_DispatchConCommand(ConCommandHandle cmdHandle, const CCommandContext &ctx, const CCommand &args)
{
	if (!g_pEntitySystem)
		return;

	auto slot = ctx.GetPlayerSlot();

	bool bSay = V_strcmp(args.Arg(0), "say");
	bool bTeamSay = V_strcmp(args.Arg(0), "say_team");

	if (slot != -1 && (bSay || bTeamSay))
	{
		auto pController = CCSPlayerController::FromSlot(slot);
		bool bFlooding = pController && pController->GetRankPlayer()->IsFlooding();

		if (*args[1] != '/' && !bFlooding)
		{
			SH_CALL(g_pCVar, &ICvar::DispatchConCommand)
			(cmdHandle, ctx, args);

			if (pController)
			{
				IGameEvent *pEvent = g_pGameEventManager->CreateEvent("player_chat");

				if (pEvent)
				{
					pEvent->SetBool("teamonly", bTeamSay);
					pEvent->SetInt("userid", pController->entindex());
					pEvent->SetString("text", args[1]);

					g_pGameEventManager->FireEvent(pEvent, true);
				}
			}
		}

		if (*args[1] == '!' || *args[1] == '/')
		{
			if (bFlooding)
				g_CChat->PrintToChat(pController, false, g_CConfig->Translate("CHAT_FLOODING"));
			else
				ParseChatCommand(args.ArgS() + 1, pController);
		}

		RETURN_META(MRES_SUPERCEDE);
	}
}

void CPlugin::AllPluginsLoaded()
{
	int ret;
	g_pMysqlClient = (IMySQLClient *)g_SMAPI->MetaFactory(MYSQLMM_INTERFACE, &ret, NULL);

	if (ret == META_IFACE_FAILED)
	{
		Fatal("Missing MYSQL plugin");
		this->ForceUnload();
	}

	g_CMysql = new CMysql();
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
	return "b2.0.1";
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
#ifdef _DEBUG
	return PLUGIN_NAME_DEBUG;
#else
	return PLUGIN_NAME;
#endif
}

const char *CPlugin::GetURL()
{
	return "https://www.verygames.net";
}

#ifdef _DEBUG

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

	pPlayer->PrintDebug(RequestType::Global);
	pPlayer->PrintDebug(RequestType::Session);
	pPlayer->PrintDebug(RequestType::Map);
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
	pPlayer->PrintDebug(RequestType::Global);

	pPlayer->m_Points.Add(10);
	pPlayer->m_DeathSuicide.Add(10);
	pPlayer->m_DeathT.Add(10);
	pPlayer->m_DeathCT.Add(10);
	pPlayer->m_BombPlanted.Add(10);
	pPlayer->m_BombExploded.Add(10);
	pPlayer->m_BombDefused.Add(10);
	pPlayer->m_KillKnife.Add(10);
	pPlayer->m_KillHeadshot.Add(10);
	pPlayer->m_KillT.Add(10);
	pPlayer->m_KillCT.Add(10);
	pPlayer->m_TeamKillT.Add(10);
	pPlayer->m_TeamKillT.Add(10);

	Debug("After SET");
	pPlayer->PrintDebug(RequestType::Global);

	Debug("Saving in database");
	pPlayer->SaveOnDatabase();
}

CON_COMMAND_EXTERN(rank_debugremove, Command_DebugRemove, "");
void Command_DebugRemove(const CCommandContext &context, const CCommand &args)
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
	pPlayer->PrintDebug(RequestType::Global);

	pPlayer->m_Points.Remove(7);
	pPlayer->m_DeathSuicide.Remove(7);
	pPlayer->m_DeathT.Remove(7);
	pPlayer->m_DeathCT.Remove(7);
	pPlayer->m_BombPlanted.Remove(7);
	pPlayer->m_BombExploded.Remove(7);
	pPlayer->m_BombDefused.Remove(7);
	pPlayer->m_KillKnife.Remove(7);
	pPlayer->m_KillHeadshot.Remove(7);
	pPlayer->m_KillT.Remove(7);
	pPlayer->m_KillCT.Remove(7);
	pPlayer->m_TeamKillT.Remove(7);
	pPlayer->m_TeamKillT.Remove(7);

	Debug("After SET");
	pPlayer->PrintDebug(RequestType::Global);

	Debug("Saving in database");
	pPlayer->SaveOnDatabase();
}

#endif