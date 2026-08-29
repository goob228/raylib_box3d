#include "SysCvar.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <raylib.h>


Cvar cv_width =     (Cvar){"width",     "width of screen",          "1200",     ACCESS_CLIENT,      CV_INT,     .valuei = 1200,    .dvaluei = 1200,     &cv_height};
Cvar cv_height =    (Cvar){"height",    "height of screen",         "800",      ACCESS_CLIENT,      CV_INT,     .valuei = 800,     .dvaluei = 800,      &cv_vsync};
Cvar cv_vsync =     (Cvar){"vsync",     "vsync toggle",             "1",        ACCESS_SERVER,      CV_BOOL,    .valueb = true,    .dvalueb = true,     &cv_FPS};
Cvar cv_FPS =       (Cvar){"FPS",       "frames per second",        "60",       ACCESS_SERVER,      CV_INT,     .valuei = 60,      .dvaluei = 60,       &cv_TPS};
Cvar cv_TPS =       (Cvar){"TPS",       "game ticks per second",    "60",       ACCESS_SERVER,      CV_INT,     .valuei = 60,      .dvaluei = 60,       NULL};

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