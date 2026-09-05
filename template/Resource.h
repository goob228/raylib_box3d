#ifndef RESOURCE_H
#define RESOURCE_H

#include <box3d/box3d.h>
#include <raylib.h> 

#define RESOURCE_NAME_SIZE 64

extern b3WorldId g_worldid;

typedef enum {
    RES_NONE,
    RES_MODEL,
    RES_TEXTURE,
} Res_type;

typedef struct {
    char name[RESOURCE_NAME_SIZE];
    int id;
} Resource_key;

Resource_key loadTextureResource(const char* name);

Resource_key setTextureResource(Texture texture, const char* name);

Texture getTextureResource(Resource_key* key);

Resource_key loadModelResource(const char* name);

Resource_key setModelResource(Model model, const char* name);

Model getModelResource(Resource_key* key);

void setUsageResource(Resource_key* key, int usage);

void initResources();

void clearResources();

#endif //RESOURCE_H