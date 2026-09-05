#include "MapLoader.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include <raylib.h>

#include "model_shared.h"


model_t loadMyMap(const char* filename)
{
    unsigned char* dataEnd = NULL;
    int dataSize = 0;
    unsigned char* data = LoadFileData(filename, &dataSize); //getRawData(filename, &dataSize);
    dataEnd = data + dataSize;

    if (!data) (model_t){0};

    int magic = *((int*)data);

    model_t mod = {0};


    strncpy(mod.name, filename, 128);
    

    if (magic == BSPVERSION) {
        TraceLog(LOG_INFO, "MapLoader.c: version == 29");
    } else if (magic == BSP2VERSION) {
        mod.isbsp2 = true;
        TraceLog(LOG_INFO, "MapLoader.c: version == BSP2");
    } else if (magic == HLBSPVERSION){
        mod.ishlbsp = true;
        TraceLog(LOG_INFO, "MapLoader.c: version == 30 (Half-life)");
    } else {
        TraceLog(LOG_ERROR, "MapLoader.c: Wrong BSP version");
        free(data);
        return (model_t){0};
    }

    TraceLog(LOG_INFO, "MapLoader.c: loaded successfully!");

    

    

    loadBSP(&mod, data, dataEnd);


    free(data);




    return mod;
}