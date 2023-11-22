#include "chat.h"
#include "utils/module.h"
#include "abstract.h"
#include "entity/ccsplayercontroller.h"
#include <string>
#include "addresses.h"
#include "config.h"

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

	char buffer[256];
	UTIL_Format(buffer, sizeof(buffer), PREFIX "%s", buf);

	std::string colorizedBuf = this->Colorizer(buffer);

	g_CAddresses->UTIL_ClientPrintAll(HUD_PRINTTALK, colorizedBuf.c_str(), nullptr, nullptr, nullptr, nullptr);
}

void CChat::PrintToChat(CPlayerSlot slot, bool canBeIgnore, const char *msg, ...)
{
	//CEntityIndex index = (CEntityIndex)(slot.Get() + 1);
	//CCSPlayerController *pController = (CCSPlayerController *)g_pEntitySystem->GetBaseEntity(index);
	CCSPlayerController *pController = CCSPlayerController::FromSlot(slot);

	if (!pController)
		return;

	CRankPlayer *pPlayer = pController->GetRankPlayer();
	if (pPlayer && (pPlayer->IsIgnoringAnnouce() && canBeIgnore))
		return;

	va_list args;
	va_start(args, msg);

	char buf[256];
	V_vsnprintf(buf, sizeof(buf), msg, args);

	va_end(args);

	char buffer[256];
	UTIL_Format(buffer, sizeof(buffer), PREFIX "%s", buf);

	std::string colorizedBuf = this->Colorizer(buffer);

	g_CAddresses->ClientPrint(pController, HUD_PRINTTALK, colorizedBuf.c_str(), nullptr, nullptr, nullptr, nullptr);
}

void CChat::PrintToChat(CBasePlayerController *player, bool canBeIgnore, const char *msg, ...)
{
	CCSPlayerController *pController = (CCSPlayerController *)player;

	if (!pController)
		return;

	CRankPlayer *pPlayer = pController->GetRankPlayer();
	if (pPlayer && (pPlayer->IsIgnoringAnnouce() && canBeIgnore))
		return;

	va_list args;
	va_start(args, msg);

	char buf[256];
	V_vsnprintf(buf, sizeof(buf), msg, args);

	va_end(args);

	char buffer[256];
	UTIL_Format(buffer, sizeof(buffer), PREFIX "%s", buf);

	std::string colorizedBuf = this->Colorizer(buffer);

	g_CAddresses->ClientPrint(pController, HUD_PRINTTALK, colorizedBuf.c_str(), nullptr, nullptr, nullptr, nullptr);
}

void CChat::PrintToChatCT(bool canBeIgnore, const char *msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[256];
	V_vsnprintf(buf, sizeof(buf), msg, args);

	va_end(args);

	this->PrintToChatTeam(CS_TEAM_CT, canBeIgnore, buf);
}

void CChat::PrintToChatT(bool canBeIgnore, const char *msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[256];
	V_vsnprintf(buf, sizeof(buf), msg, args);

	va_end(args);

	this->PrintToChatTeam(CS_TEAM_T, canBeIgnore, buf);
}

void CChat::PrintToChatTeam(int teamIndex, bool canBeIgnore, const char *msg)
{
	for (int i = 0; i < 64; i++)
	{
		//CBaseEntity2 *player = (CBaseEntity2 *)g_pEntitySystem->GetBaseEntity((CEntityIndex)(i + 1));
		CCSPlayerController *player = CCSPlayerController::FromSlot(i);
		if (!player)
			continue;

		if (player->m_iTeamNum.Get() != teamIndex)
			continue;

		this->PrintToChat(i, canBeIgnore, msg);
	}
}
