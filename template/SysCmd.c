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


Command cmd_help =      (Command){"help",       "list all commands",            NULL, NULL, NULL, &cmd_cvars};
Command cmd_cvars =     (Command){"cvars",      "list all console variables",   NULL, NULL, NULL, &cmd_map};
Command cmd_map =       (Command){"map",        "load map from /res folder",    NULL, NULL, NULL, &cmd_textures};
Command cmd_textures =  (Command){"textures",   "list all available textures",  NULL, NULL, NULL, &cmd_exit};
Command cmd_exit =      (Command){"exit",       "exit the game",                NULL, NULL, NULL, NULL};


Command* getFirstCommand()
{
    return &cmd_help;
}

void setCommandCallBack(Command* cmd, void (*callback)(Command*))
{
    if (cmd) cmd->callback = callback;
}