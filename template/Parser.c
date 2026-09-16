#include "Parser.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <raylib.h>

#include "SysCvar.h"
#include "SysCmd.h"

#define LINE_SIZE 128
#define LINES_COUNT_MAX 4

static char sent[LINES_COUNT_MAX][LINE_SIZE];

void parseString(const char* text, size_t len)
{
    for (int i = 0; i < LINES_COUNT_MAX; i++) {
        sent[i][0] = 0;
    }

    int lines_count = 0;
    int letters_count = 0;
    for (size_t i = 0; i < len, text[i]; i++) {

        switch (text[i])
        {
        case ' ':
            if (sent[lines_count][0]) {
                sent[lines_count][letters_count] = 0;
                letters_count = 0;
                lines_count++;
                if (lines_count >= LINES_COUNT_MAX) {
                    TraceLog(LOG_ERROR, "Parser.c: too many words: %s", text);
                    return;
                }
            }
            break;
        
        default:
            sent[lines_count][letters_count] = text[i];
            letters_count++;
            if (letters_count>=LINE_SIZE-1) {
                TraceLog(LOG_ERROR, "Parser.c: too many letters in one word: %s", text);
                return;
            }
            break;
        }

    }

    sent[lines_count][letters_count] = 0;

    if (!sent[0][0]) return;


    Command* curcmd = getFirstCommand();
    
    do {
        if (strncmp(curcmd->name, sent[0], LINE_SIZE) == 0) {
            execCommand(curcmd);
            return;
        }
    } while (curcmd = curcmd->next);

    Cvar* curvar = getFirstCvar();
    
    do {
        if (strncmp(curvar->name, sent[0], LINE_SIZE) == 0) {
            parseCvarValues(curvar, sent[1]);
            printCvar(curvar);
            return;
        }
    } while (curvar = curvar->next);

    TraceLog(LOG_WARNING, "Parser.c: couldnt find cvar or command with name: %s", sent[0]);

}