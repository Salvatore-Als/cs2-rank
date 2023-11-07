#include "abstract.h"
#include "basecommands.h"
#include "chat.h"
#include "entity/ccsplayercontroller.h"

CON_COMMAND_CHAT(rank, "Display your rank")
{
    if (!player)
    {
        Debug("Invalid player");
        return;
    }

    g_CChat->PrintToChat(player, "Command works ");
}