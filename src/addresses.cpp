#include "abstract.h"
#include "utils/module.h"
#include "addresses.h"
#include "tier0/memdbgon.h"

#define RESOLVE_SIG(gameConfig, name, variable) variable = (decltype(variable))gameConfig->ResolveSignature(name)

void CAddresses::Init(CGameConfig *g_CGameConfig)
{
    g_moduleServer = new CModule(GAMEBIN, "server");
    g_moduleEngine = new CModule(ROOTBIN, "engine2");
    g_schemasystem = new CModule(ROOTBIN, "schemasystem");

    RESOLVE_SIG(g_CGameConfig, "NetworkStateChanged", CAddresses::NetworkStateChanged);
    RESOLVE_SIG(g_CGameConfig, "StateChanged", CAddresses::StateChanged);
    RESOLVE_SIG(g_CGameConfig, "ClientPrint", CAddresses::ClientPrint);
    RESOLVE_SIG(g_CGameConfig, "UTIL_ClientPrintAll", CAddresses::UTIL_ClientPrintAll);
}

void CAddresses::Clean()
{
    delete g_moduleServer;
    delete g_moduleEngine;
    delete g_schemasystem;
}