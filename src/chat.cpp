#include "chat.h"
#include "utils/module.h"
#include "abstract.h"
#include "entity/cbaseentity.h"

void CChat::PrintToChatAll(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[256];
	V_vsnprintf(buf, sizeof(buf), msg, args);

	va_end(args);

	g_CAddresses->UTIL_ClientPrintAll(HUD_PRINTTALK, buf, nullptr, nullptr, nullptr, nullptr);
}

void CChat::PrintToChat(CBasePlayerController *player, const char *msg, ...)
{
	if (!player)
	{
		return;
	}

	va_list args;
	va_start(args, msg);

	char buf[256];
	V_vsnprintf(buf, sizeof(buf), msg, args);

	va_end(args);

	g_CAddresses->ClientPrint(player, HUD_PRINTTALK, buf, nullptr, nullptr, nullptr, nullptr);
}

void CChat::PrintToChat(CPlayerSlot slot, const char *msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[256];
	V_vsnprintf(buf, sizeof(buf), msg, args);

	va_end(args);

	CEntityIndex index = (CEntityIndex)(slot.Get() + 1);
	CBasePlayerController *player = (CBasePlayerController *)g_pEntitySystem->GetBaseEntity(index);

	g_CAddresses->ClientPrint(player, HUD_PRINTTALK, buf, nullptr, nullptr, nullptr, nullptr);
}

void CChat::PrintToChatCT(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[256];
	V_vsnprintf(buf, sizeof(buf), msg, args);

	va_end(args);

	PrintToChatTeam(CS_TEAM_CT, buf);
}

void CChat::PrintToChatT(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[256];
	V_vsnprintf(buf, sizeof(buf), msg, args);

	va_end(args);

	PrintToChatTeam(CS_TEAM_T, buf);
}

void CChat::PrintToChatTeam(int teamIndex, const char *msg)
{
	for (int i = 0; i < 64; i++)
	{
		CBaseEntity2 *player = (CBaseEntity2 *)g_pEntitySystem->GetBaseEntity((CEntityIndex)(i + 1));
		if (!player)
		{
			continue;
		}

		if (player->m_iTeamNum.Get() != teamIndex)
		{
			continue;
		}

		PrintToChat(i, msg);
	}
}