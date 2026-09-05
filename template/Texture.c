#include "Texture.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>

#include <raylib.h>

#include "model_shared.h"
#include "Zone.h"


Texture loadLmpTexture(const char* filename, int* success) {

    int mark = Hunk_LowMark();
    unsigned char* data = W_GetTextureBGRA(filename);
    if (data) {
        
        Image img = (Image){0};
        img.data = data;
        img.width = model_shared_image_width;
        img.height = model_shared_image_height;
        img.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        img.mipmaps = 1;

        Texture texture = LoadTextureFromImage(img);
        *success = 1;
        Hunk_FreeToLowMark(mark);
        return texture;

    }
    *success = 0;
    Hunk_FreeToLowMark(mark);
    return (Texture){0};
}
