#include "MapLoader.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include <raylib.h>

#include "model_shared.h"




void* getRawData(const char* filename, size_t* outputFileSize)
{
    void* data = NULL;
    FILE* fp = fopen(filename, "rb");

    if (fp == NULL) {
        TraceLog(LOG_ERROR,"MapLoader.c: File doesnt exist" );
        return NULL;
    }
    
    
    if (fseek(fp, 0, SEEK_END) != 0) {
        TraceLog(LOG_ERROR, "MapLoader.c: fseek failed");
        fclose(fp);
        return NULL;
    }

    size_t fileSize = ftell(fp);

    if (fileSize == -1L) {
        TraceLog(LOG_ERROR, "MapLoader.c: ftell failed");
        fclose(fp);
        return NULL;
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        TraceLog(LOG_ERROR, "MapLoader.c: fseek failed");
        fclose(fp);
        return NULL;
    }

    data = malloc(fileSize);

    if (data == NULL) {
        TraceLog(LOG_ERROR, "MapLoader.c: malloc failed");
        fclose(fp);
        return NULL;
    }

    size_t bytesRead = fread(data,1,fileSize,fp);

    if (bytesRead < fileSize) {
        TraceLog(LOG_ERROR, "MapLoader.c: fread failed");
        fclose(fp);
        free(data);
        return NULL;
    }

    fclose(fp);
    *outputFileSize = fileSize;
    return data;
}


model_t* loadMyMap(const char* filename)
{
    void* dataEnd = NULL;
    size_t dataSize = 0;
    void* data = getRawData(filename, &dataSize);
    dataEnd = data + dataSize;

    if (!data) return NULL;

    int magic = *((int*)data);

    model_t mod = {0};


    strncpy(mod.name, filename, 128);
    

    if (magic == BSPVERSION) {
        TraceLog(LOG_INFO, "MapLoader.c: version == 29");
    } else if (magic == BSP2VERSION) {
        mod.isbsp2 = true;
        TraceLog(LOG_INFO, "MapLoader.c: version == BSP2");
    } else {
        TraceLog(LOG_ERROR, "MapLoader.c: Wrong BSP version");
        free(data);
        return NULL;
    }

    TraceLog(LOG_INFO, "MapLoader.c: loaded successfully!");

    

    

    loadBSP(&mod, data, dataEnd);


    free(data);


    model_t* mesh = (model_t*)malloc(sizeof(model_t));

    memcpy(mesh, &mod, sizeof(*mesh));


    return mesh;
}