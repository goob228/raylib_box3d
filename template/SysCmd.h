#ifndef SYSCMD_H
#define SYSCMD_H

#include <stdbool.h>

typedef struct Command {
    const char* name;
    const char* decription;
    
    void (*callback)(struct Command*);
    void* data;
    void* data2;

    struct Command* next;
    
} Command;

bool execCommand(Command* cmd);

void setCommandCallBack(Command* cmd, void (*callback)(Command*));

Command* getFirstCommand();

extern Command cmd_help;
extern Command cmd_cvars;
extern Command cmd_map;
extern Command cmd_textures;
extern Command cmd_flush;
extern Command cmd_hunk_print;
extern Command cmd_exit;

void initCmds();


#endif //SYSCMD_H
