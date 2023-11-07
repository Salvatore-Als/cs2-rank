#include "abstract.h"
#include "basecommands.h"
#include "chat.h"
#include "entity/ccsplayercontroller.h"

CON_COMMAND_CHAT(rank, "Display your rank")
{
    if (!player)
    {
        return;
    }

    g_CChat->PrintToChat(player, "Command works ");
}

CON_COMMAND_CHAT(sessions, "Display your rank")
{
    if (!player)
    {
        return;
    }

    g_CChat->PrintToChat(player, "Command works ");
}

CON_COMMAND_CHAT(stats, "Display your rank")
{
    if (!player)
    {
        return;
    }

    g_CChat->PrintToChat(player, "Command works ");
}

CON_COMMAND_CHAT(top, "Display your rank")
{
    if (!player)
    {
        return;
    }

    g_CChat->PrintToChat(player, "Command works ");
}