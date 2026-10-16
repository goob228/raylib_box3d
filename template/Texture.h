#ifndef TEXTURE_H
#define TEXTURE_H


#include <raylib.h>



Texture loadLmpTexture(const char* filename, int* success);


void loadPalette(const char* filename);

void W_LoadTextureWadFile (char *filename, int complain);



#endif //TEXTURE_H