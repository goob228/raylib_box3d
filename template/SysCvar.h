#ifndef SYSCVAR_H
#define SYSCVAR_H

#include <stdbool.h>

#define CVAR_STRING_SIZE 16

typedef enum {
    CV_BOOL,
    CV_FLOAT,
    CV_INT,
    CV_STRING,

} Cvar_value_type;

typedef enum {
    ACCESS_CHEATS,
    ACCESS_SERVER,
    ACCESS_CLIENT
} Cvar_access_type;

typedef union {
    bool valb;
    float valf;
    int vali;
} Cvar_value;

typedef struct Cvar {
    const char* name;
    const char* description;

    char value[CVAR_STRING_SIZE];

    Cvar_access_type access_type;

    Cvar_value_type value_type;

    union {
        bool valueb;
        float valuef;
        int valuei;
    };

    union {
        bool dvalueb;
        float dvaluef;
        int dvaluei;
    };

    struct Cvar* next;

    void (*callback)(struct Cvar*);
    void* data;

} Cvar;

Cvar* getFirstCvar();

void setCvarValue(Cvar* cvar, Cvar_value v_u);

void printCvar(Cvar* cvar);

void setCvarCallback(Cvar* cvar, void (*callback)(Cvar*));


extern Cvar cv_width;
extern Cvar cv_height;
extern Cvar cv_vsync;
extern Cvar cv_FPS;
extern Cvar cv_TPS;


#endif //SYSCVAR_H