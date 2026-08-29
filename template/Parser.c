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
            if (sent[1][0]) {
                char* endptr = NULL;
                switch (curvar->value_type)
                {
                case CV_BOOL:
                    if ((sent[1][0] == '1' || sent[1][0] == '0') && sent[1][1] == 0) {
                        bool new = (sent[1][0] == '1') ? true : false;
                        setCvarValue(curvar, (Cvar_value){.valb = new});
                    }
                    break;

                case CV_FLOAT:
                    char* endptr1 = sent[1];
                    float newf = strtof(sent[1], &endptr1);
                    if (*endptr1 == 0) {
                        setCvarValue(curvar, (Cvar_value){.valf = newf});
                    }
                    break;

                case CV_INT:
                    char* endptr2 = sent[1];
                    
                    int newi = (int)strtol(sent[1], &endptr2, 10);
                    if (*endptr2 == 0) {
                        setCvarValue(curvar, (Cvar_value){.vali = newi});
                    }
                    break;

                case CV_STRING:
                    strncpy(curvar->value, sent[1], CVAR_STRING_SIZE);
                    break;
                
                default:
                    TraceLog(LOG_ERROR, "Parser.c: invalid values in %s", text);
                    return;
                    break;
                }
            }
            printCvar(curvar);
            return;
        }
    } while (curvar = curvar->next);

    TraceLog(LOG_WARNING, "Parser.c: couldnt find cvar or command with name: %s", sent[0]);

}