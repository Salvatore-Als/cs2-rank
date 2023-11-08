#include "chat.h"
#include "utils/module.h"
#include "abstract.h"
#include "entity/cbaseentity.h"
#include <string>
#include "addresses.h"

std::string CChat::Colorizer(std::string str)
{
	for (int i = 0; i < std::size(colorsHex); i++)
	{
		size_t pos = 0;

		while ((pos = str.find(colorsString[i], pos)) != std::string::npos)
		{
			str.replace(pos, colorsString[i].length(), colorsHex[i]);
			pos += colorsHex[i].length();
		}
	}

	return str;
}

void CChat::PrintToChatAll(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[256];
	V_vsnprintf(buf, sizeof(buf), msg, args);

	va_end(args);

	std::string colorizedBuf = this->Colorizer(buf);

	g_CAddresses->UTIL_ClientPrintAll(HUD_PRINTTALK, colorizedBuf.c_str(), nullptr, nullptr, nullptr, nullptr);
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

	std::string colorizedBuf = this->Colorizer(buf);

	g_CAddresses->ClientPrint(player, HUD_PRINTTALK, colorizedBuf.c_str(), nullptr, nullptr, nullptr, nullptr);
}

void CChat::PrintToChat(CBasePlayerController *player, const char *msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[256];
	V_vsnprintf(buf, sizeof(buf), msg, args);

	va_end(args);

	std::string colorizedBuf = this->Colorizer(buf);

	g_CAddresses->ClientPrint(player, HUD_PRINTTALK, colorizedBuf.c_str(), nullptr, nullptr, nullptr, nullptr);
}

void CChat::PrintToChatCT(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[256];
	V_vsnprintf(buf, sizeof(buf), msg, args);

	va_end(args);

	this->PrintToChatTeam(CS_TEAM_CT, buf);
}

void CChat::PrintToChatT(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[256];
	V_vsnprintf(buf, sizeof(buf), msg, args);

	va_end(args);

	this->PrintToChatTeam(CS_TEAM_T, buf);
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

		this->PrintToChat(i, msg);
	}
}
