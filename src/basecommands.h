#pragma once
#include "entity/ccsplayercontroller.h"
#include "convar.h"

#define COMMAND_PREFIX "mm_"

typedef void (*FnChatCommandCallback_t)(const CCommand &args, CCSPlayerController *player);

class CChatCommand;
// CUtlMap<uint32, CChatCommand *> g_Commands(0, 0, DefLessFunc(uint32));
extern CUtlMap<uint32, CChatCommand *> g_Commands;

class CChatCommand
{
public:
	CChatCommand(const char *cmd, FnChatCommandCallback_t callback, const char *description) : m_pfnCallback(callback), m_szName(cmd), m_szDescription(description)
	{
		g_Commands.Insert(hash_32_fnv1a_const(cmd), this);
	}

	const char *GetName() const { return m_szName; }
	const char *GetDescription() const { return m_szDescription; }

	void operator()(const CCommand &args, CCSPlayerController *player)
	{
		if (player && !player->IsConnected())
		{
			return;
		}

		m_pfnCallback(args, player);
	}

private:
	FnChatCommandCallback_t m_pfnCallback;
	const char* m_szName;
	const char* m_szDescription;
};

void ParseChatCommand(const char *, CCSPlayerController *);

#define CON_COMMAND_CHAT(name, description)                                                                                         \
	void name##_callback(const CCommand &args, CCSPlayerController *player);                                                        \
	static CChatCommand name##_chat_command(#name, name##_callback, description);                                                   \
	static void name##_con_callback(const CCommandContext &context, const CCommand &args)                                           \
	{                                                                                                                               \
		CCSPlayerController *pController = nullptr;                                                                                 \
		if (context.GetPlayerSlot().Get() != -1)                                                                                    \
			pController = (CCSPlayerController *)g_pEntitySystem->GetBaseEntity((CEntityIndex)(context.GetPlayerSlot().Get() + 1)); \
                                                                                                                                    \
		name##_chat_command(args, pController);                                                                                     \
	}                                                                                                                               \
	static ConCommandRefAbstract name##_ref;                                                                                        \
	static ConCommand name##_command(&name##_ref, COMMAND_PREFIX #name, name##_con_callback,                                        \
									 description, FCVAR_CLIENT_CAN_EXECUTE | FCVAR_LINKED_CONCOMMAND);                              \
	void name##_callback(const CCommand &args, CCSPlayerController *player)