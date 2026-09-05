#include "SysCmd.h" 

#include <stdbool.h>
#include <stdint.h>

#include <raylib.h>


bool execCommand(Command* cmd)
{
    if (cmd) {
        if (cmd->callback) {
            cmd->callback(cmd);
            return true;
        } else {
            TraceLog(LOG_WARNING, "SysCmd.c: no callback attached to %s", cmd->name);
        }
    }

    return false;
}


Command cmd_help =      {"help",       "list all commands",            NULL, NULL, NULL, NULL};
Command cmd_cvars =     {"cvars",      "list all console variables",   NULL, NULL, NULL, NULL};
Command cmd_map =       {"map",        "load map from /res folder",    NULL, NULL, NULL, NULL};
Command cmd_textures =  {"textures",   "list all available textures",  NULL, NULL, NULL, NULL};
Command cmd_flush =     {"flush",      "Throw everything out, so new data will be demand cached",  NULL, NULL, NULL, NULL};
Command cmd_hunk_print ={"hunk_print", "prints allocations",           NULL, NULL, NULL, NULL};
Command cmd_exit =      {"exit",       "exit the game",                NULL, NULL, NULL, NULL};

void initCmds() 
{
    cmd_help.next = &cmd_cvars;
    cmd_cvars.next = &cmd_map;
    cmd_map.next = &cmd_textures;
    cmd_textures.next = &cmd_flush;
    cmd_flush.next = &cmd_hunk_print;
    cmd_hunk_print.next = &cmd_exit;
}

Command* getFirstCommand()
{
    return &cmd_help;
}

void setCommandCallBack(Command* cmd, void (*callback)(Command*))
{
    if (cmd) cmd->callback = callback;
}

