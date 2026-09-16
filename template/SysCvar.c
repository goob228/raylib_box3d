#include "SysCvar.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <raylib.h>



Cvar cv_width =     {"width",     "width of screen",          "1200",       ACCESS_CLIENT,          CV_INT};
Cvar cv_height =    {"height",    "height of screen",         "800",        ACCESS_CLIENT,          CV_INT};
Cvar cv_vsync =     {"vsync",     "vsync toggle",             "1",          ACCESS_SERVER,          CV_BOOL};
Cvar cv_FPS =       {"FPS",       "frames per second",        "170",        ACCESS_SERVER,          CV_INT};
Cvar cv_TPS =       {"TPS",       "game ticks per second",    "50",         ACCESS_SERVER,          CV_INT};
Cvar cv_wireframe = {"wire",      "enable wireframe mode",    "0",          ACCESS_SERVER,          CV_BOOL};

void printCvar(Cvar* cvar)
{
    switch (cvar->value_type)
        {
        case CV_BOOL:
            TraceLog(LOG_NONE, "%s: %s; default: %i; value: %i", cvar->name, cvar->description, cvar->dvalueb, cvar->valueb);
            break;
        
        case CV_FLOAT:
            TraceLog(LOG_NONE, "%s: %s; default: %f; value: %f", cvar->name, cvar->description, cvar->dvaluef, cvar->valuef);
            break;

        case CV_INT:
            TraceLog(LOG_NONE, "%s: %s; default: %i; value: %i", cvar->name, cvar->description, cvar->dvaluei, cvar->valuei);
            break;
        
        default:
            cvar->value[CVAR_STRING_SIZE-1] = 0;
            TraceLog(LOG_NONE, "%s: %s; value: %s", cvar->name, cvar->description, cvar->value);
            break;
        }
}

void parseCvarValues(Cvar* curvar, char* word)
{
    if (word[0]) {
        char* endptr = NULL;
        switch (curvar->value_type)
        {
        case CV_BOOL:
            if ((word[0] == '1' || word[0] == '0') && word[1] == 0) {
                bool new = (word[0] == '1') ? true : false;
                setCvarValue(curvar, (Cvar_value){.valb = new});
            }
            break;

        case CV_FLOAT:
            char* endptr1 = word;
            float newf = strtof(word, &endptr1);
            if (*endptr1 == 0) {
                setCvarValue(curvar, (Cvar_value){.valf = newf});
            }
            break;

        case CV_INT:
            char* endptr2 = word;
            
            int newi = (int)strtol(word, &endptr2, 10);
            if (*endptr2 == 0) {
                setCvarValue(curvar, (Cvar_value){.vali = newi});
            }
            break;

        case CV_STRING:
            strncpy(curvar->value, word, CVAR_STRING_SIZE);
            break;
        
        default:
            break;
        }
    }
}

Cvar* getFirstCvar()
{
    return &cv_width;
}

void setCvarValue(Cvar* cvar, Cvar_value v_u)
{
    switch (cvar->value_type)
    {
    case CV_BOOL:
        cvar->valueb = v_u.valb;
        snprintf(cvar->value, CVAR_STRING_SIZE, "%i", cvar->valueb);
        cvar->value[CVAR_STRING_SIZE-1] = 0;
        if (cvar->callback) cvar->callback(cvar);
        break;

    case CV_FLOAT:
        cvar->valuef = v_u.valf;
        snprintf(cvar->value, CVAR_STRING_SIZE, "%f", cvar->valuef);
        cvar->value[CVAR_STRING_SIZE-1] = 0;
        if (cvar->callback) cvar->callback(cvar);
        break;

    case CV_INT:
        cvar->valuei = v_u.vali;
        snprintf(cvar->value, CVAR_STRING_SIZE, "%i", cvar->valuei);
        cvar->value[CVAR_STRING_SIZE-1] = 0;
        if (cvar->callback) cvar->callback(cvar);
        break;
    
    default:
        break;
    }



}

void setCvarCallback(Cvar* cvar, void (*callback)(Cvar*))
{
    if (cvar) {
        cvar->callback = callback;
    }
}

void initCvars(){
    cv_width.next = &cv_height;
    cv_height.next = &cv_vsync;
    cv_vsync.next = &cv_FPS;
    cv_FPS.next = &cv_TPS;
    cv_TPS.next = &cv_wireframe;
    


    Cvar* curvar = getFirstCvar();
    
    do {
        parseCvarValues(curvar, curvar->value);
        switch (curvar->value_type)
        {
        case CV_BOOL:
            curvar->dvalueb = curvar->valueb;
            break;

        case CV_FLOAT:
            curvar->dvaluef = curvar->valuef;
            break;

        case CV_INT:
            curvar->dvaluei = curvar->valuei;
            break;
        
        default:
            break;
        }

    } while (curvar = curvar->next);

}