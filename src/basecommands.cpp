#include "abstract.h"
#include "basecommands.h"
#include "entity/ccsplayercontroller.h"

void ParseChatCommand(const char *pMessage, CCSPlayerController *pController)
{
    if (!pController || !pController->IsConnected())
        return;

    CCommand args;
    args.Tokenize(pMessage + 1);

    uint16 index = g_Commands.Find(hash_32_fnv1a_const(args[0]));

    if (!g_Commands.IsValidIndex(index))
        return;

    (*g_Commands[index])(args, pController);
}