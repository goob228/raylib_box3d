#include "MapLoader.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "model_shared.h"


void* getRawData(const char* filename, size_t* outputFileSize)
{
    void* data = NULL;
    FILE* fp = fopen(filename, "rb");

    if (fp == NULL) {
        perror("File doesnt exist\n");
        return NULL;
    }
    
    
    if (fseek(fp, 0, SEEK_END) != 0) {
        perror("fseek failed\n");
        fclose(fp);
        return NULL;
    }

    size_t fileSize = ftell(fp);

    if (fileSize == -1L) {
        perror("ftell failed\n");
        fclose(fp);
        return NULL;
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        perror("fseek failed\n");
        fclose(fp);
        return NULL;
    }

    data = malloc(fileSize);

    if (data == NULL) {
        perror("malloc failed\n");
        fclose(fp);
        return NULL;
    }

    size_t bytesRead = fread(data,1,fileSize,fp);

    if (bytesRead < fileSize) {
        perror("fread failed\n");
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


    strcpy(mod.name, filename);
    

    if (magic == BSPVERSION) {
        printf("version == 29\n");
    } else if (magic == BSP2VERSION) {
        mod.isbsp2 = true;
        printf("version == BSP2\n");
    } else {
        perror("Wrong BSP version\n");
        free(data);
        return NULL;
    }

    printf("BSP loaded successfully!\n");

    

    

    loadBSP(&mod, data, dataEnd);


    free(data);

    printf("Ended without seg fault\n");

    model_t* mesh = (model_t*)malloc(sizeof(model_t));

    memcpy(mesh, &mod, sizeof(*mesh));


    return mesh;
}