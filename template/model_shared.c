#include "model_shared.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>

#define STB_DXT_IMPLEMENTATION
#include <stb_dxt.h>
#include <raylib.h>

#include "Zone.h"
#include "G_local.h"
#include "Resource.h"

#include "QuakePalette.h"

#define PRINT(val, ...) TraceLog(LOG_WARNING, "model_shared.c: " val, ##__VA_ARGS__)

#define LittleLong(l) BuffLittleLong((unsigned char *)&(l))




static model_t loadmodel;

#define MAX_WAD_COUNT 64

static struct {mwad_t w[MAX_WAD_COUNT]; int numwads;} wads = {0};

int model_shared_image_width, model_shared_image_height;

unsigned char model_shared_texture_name[17];

int pixel_format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

typedef struct {
	int mx;
	int my;
	int width;
	int height;
	int curh;

} lightmap_state;

typedef struct {
	float x, y, z;
} vec3;


typedef struct {
	char lumpname[24]; // up to 23 chars, zero-padded
	int fileofs;  // from file start
	int filelen;
} bspx_lump_t;
typedef struct {
	char id[4];  // 'BSPX'
	int numlumps;
	bspx_lump_t lumps[1];
} bspx_header_t;

bspx_lump_t* BSPX_FindLump(bspx_header_t *bspxheader, char* lumpname);
bspx_header_t *BSPX_Setup(char *filebase, size_t filelen, size_t lumpsEnd);

void Mod_BSPX_LoadShifts(bspx_header_t *bspxheader, unsigned char* filedata);
void Mod_BSPX_LoadRGBLighting(bspx_header_t *bspxheader, unsigned char* filedata);

vec3 swapyz(vec3 p)
{
	vec3 p2 = (vec3){-p.x, p.z, p.y};
	return p2;
}

vec3 cross_product(vec3 p1, vec3 p2)
{
	return (vec3){p1.y * p2.z, p1.x * p2.z, p1.x * p2.y};
}

float dot_product(vec3 p1, vec3 p2)
{
	return (p1.x * p2.x + p1.y * p2.y + p1.z * p2.z);
}

float Q_rsqrt( float number )
{
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y  = number;
	i  = * ( long * ) &y;                       // evil floating point bit level hacking
	i  = 0x5f3759df - ( i >> 1 );               // what the fuck?
	y  = * ( float * ) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

	return y;
}

vec3 calc_normal(vec3 p1, vec3 p2, vec3 p3)
{
	vec3 d1 = (vec3){p2.x - p1.x, p2.y - p1.y, p2.z - p1.z};
	vec3 d2 = (vec3){p3.x - p1.x, p3.y - p1.y, p3.z - p1.z};
	vec3 normal = cross_product(d1, d2);
	float length = dot_product(normal, normal);
	if (length) {
		float rsq = Q_rsqrt(length);
		normal.x *= rsq;
		normal.y *= rsq;
		normal.z *= rsq;
	}

	return normal;
}

void MSG_BeginReading(sizebuf_t *sb)
{
	sb->readcount = 0;
	sb->badread = false;
}

void MSG_InitReadBuffer (sizebuf_t *buf, unsigned char *data, int size)
{
	memset(buf, 0, sizeof(*buf));
	buf->data = data;
	buf->maxsize = buf->cursize = size;
	MSG_BeginReading(buf);
}


#define MSG_ReadByte_opt(sb) ((sb)->readcount >= (sb)->cursize ? ((sb)->badread = true, '\0') : (unsigned char)(sb)->data[(sb)->readcount++])

#define MSG_ReadByte(sb) ((sb)->readcount >= (sb)->cursize ? ((sb)->badread = true, -1) : (unsigned char)(sb)->data[(sb)->readcount++])

static int BuffLittleLong (const unsigned char *buffer)
{
	return ((unsigned)buffer[3] << 24) | (buffer[2] << 16) | (buffer[1] << 8) | buffer[0];
}

int MSG_ReadLittleLong (sizebuf_t *sb)
{
	if (sb->readcount+4 > sb->cursize)
	{
		sb->badread = true;
		return -1;
	}
	sb->readcount += 4;
	return sb->data[sb->readcount-4] | (sb->data[sb->readcount-3]<<8) | (sb->data[sb->readcount-2]<<16) | ((unsigned)sb->data[sb->readcount-1]<<24);
}

short BuffLittleShort (const unsigned char *buffer)
{
	return (buffer[1] << 8) | buffer[0];
}

int MSG_ReadLittleShort(sizebuf_t *sb)
{
	if (sb->readcount+2 > sb->cursize)
	{
		sb->badread = true;
		return -1;
	}   
	sb->readcount += 2;
	return (short)(sb->data[sb->readcount-2] | (sb->data[sb->readcount-1]<<8));
}

float MSG_ReadLittleFloat (sizebuf_t *sb)
{
	union
	{
		float f;
		int l;
	} dat;
	if (sb->readcount+4 > sb->cursize)
	{
		sb->badread = true;
		return -1;
	}
	sb->readcount += 4;
	dat.l = sb->data[sb->readcount-4] | (sb->data[sb->readcount-3]<<8) | (sb->data[sb->readcount-2]<<16) | ((unsigned)sb->data[sb->readcount-1]<<24);
	return dat.f;
}

size_t MSG_ReadBytes (sizebuf_t *sb, size_t numbytes, unsigned char *out)
{
	size_t l = 0;

	// when numbytes have been read sb->readcount won't be advanced any further
	while (l < numbytes && !sb->badread)
		out[l++] = MSG_ReadByte_opt(sb);
	return l;
}



static void W_CleanupName (const char *in, char *out)
{
	int		i;
	int		c;

	for (i=0 ; i<16 ; i++ )
	{
		c = in[i];
		if (!c)
			break;

		if (c >= 'A' && c <= 'Z')
			c += ('a' - 'A');
		out[i] = c;
	}

	for ( ; i< 16 ; i++ )
		out[i] = 0;

}

static void W_SwapLumps(int numlumps, lumpinfo_t *lumps)
{
	int i;
	for (i = 0;i < numlumps;i++)
	{
		lumps[i].filepos = LittleLong(lumps[i].filepos);
		lumps[i].disksize = LittleLong(lumps[i].disksize);
		lumps[i].size = LittleLong(lumps[i].size);
		W_CleanupName(lumps[i].name, lumps[i].name);
	}
}

/*
====================
W_LoadTextureWadFile
====================
*/
void W_LoadTextureWadFile (char *filename, int complain)
{
	if (wads.numwads >= MAX_WAD_COUNT-1) {
		TraceLog(LOG_WARNING, "model_shared.c: Wad counter out of range %i", wads.numwads);
		return;
	}

	wadinfo_t		header;
	int				infotableofs;
	FILE			*file;
	int				numlumps;
	mwad_t			w;

	w.version = 2;

	file = fopen(filename, "rb");
	if (!file)
	{
		if (complain)
			TraceLog(LOG_ERROR, "model_shared.c: W_LoadTextureWadFile: couldn't find %s\n", filename);
		return;
	}

	if (fread(&header, 1, sizeof(wadinfo_t), file) != sizeof(wadinfo_t))
	{TraceLog(LOG_ERROR, "model_shared.c: W_LoadTextureWadFile: unable to read wad header\n");fclose(file);file = NULL;return;}

	if(!memcmp(header.identification, "WAD2", 4)){

	} else if (!memcmp(header.identification, "WAD3", 4)) {
		w.version = 3;
	} else 
	{TraceLog(LOG_ERROR, "model_shared.c: W_LoadTextureWadFile: Wad file %s doesnt have WAD2 or WAD3 id\n",filename);fclose(file);file = NULL;return;}

	numlumps = LittleLong(header.numlumps);
	if (numlumps < 1 || numlumps > 65536)
	{TraceLog(LOG_ERROR, "model_shared.c: W_LoadTextureWadFile: invalid number of lumps (%i)\n", numlumps);fclose(file);file = NULL;return;}
	infotableofs = LittleLong(header.infotableofs);
	if (fseek(file, infotableofs, SEEK_SET))
	{TraceLog(LOG_ERROR, "model_shared.c: W_LoadTextureWadFile: unable to seek to lump table\n");fclose(file);file = NULL;return;}


	w.file = file;
	w.numlumps = numlumps;
	w.lumps = (lumpinfo_t *)malloc(w.numlumps * sizeof(lumpinfo_t)); //Mem_Alloc(cls.permanentmempool, w->numlumps * sizeof(lumpinfo_t));

	if (!w.lumps)
	{
		TraceLog(LOG_ERROR, "model_shared.c: W_LoadTextureWadFile: unable to allocate temporary memory for lump table\n");
		fclose(w.file);
		w.file = NULL;
		w.numlumps = 0;
		return;
	}

	if (fread(w.lumps, 1, sizeof(lumpinfo_t) * w.numlumps, file) != (fs_offset_t)sizeof(lumpinfo_t) * numlumps)
	{
		TraceLog(LOG_ERROR, "model_shared.c: W_LoadTextureWadFile: unable to read lump table\n");
		fclose(w.file);
		w.file = NULL;
		w.numlumps = 0;
		free(w.lumps);
		w.lumps = NULL;
		return;
	}

	W_SwapLumps(w.numlumps, w.lumps);

    wads.w[wads.numwads] = w;
	if (wads.numwads < MAX_WAD_COUNT-1) {
		wads.numwads++;
	}

	// leaves the file open
}

unsigned char *W_ConvertWAD2TextureRGBA(sizebuf_t *sb)
{
	unsigned char *in, *data, *out, *pal;
	int d, p;
	unsigned char name[16];
	unsigned int mipoffset[4];

	MSG_BeginReading(sb);
	MSG_ReadBytes(sb, 16, model_shared_texture_name);
	model_shared_texture_name[16] = 0;
	model_shared_image_width = MSG_ReadLittleLong(sb);
	model_shared_image_height = MSG_ReadLittleLong(sb);
	mipoffset[0] = MSG_ReadLittleLong(sb);
	mipoffset[1] = MSG_ReadLittleLong(sb); // should be mipoffset[0] + model_shared_image_width*model_shared_image_height
	mipoffset[2] = MSG_ReadLittleLong(sb); // should be mipoffset[1] + model_shared_image_width*model_shared_image_height/4
	mipoffset[3] = MSG_ReadLittleLong(sb); // should be mipoffset[2] + model_shared_image_width*model_shared_image_height/16
	//pal = sb->data + mipoffset[3] + (model_shared_image_width / 8 * model_shared_image_height / 8) + 2;
	pal = (unsigned char*)Palette;

	// bail if any data looks wrong
	if (model_shared_image_width < 0
	 || model_shared_image_width > 4096
	 || model_shared_image_height < 0
	 || model_shared_image_height > 4096
	 || mipoffset[0] != 40
	 || mipoffset[1] != mipoffset[0] + model_shared_image_width * model_shared_image_height
	 || mipoffset[2] != mipoffset[1] + model_shared_image_width / 2 * model_shared_image_height / 2
	 || mipoffset[3] != mipoffset[2] + model_shared_image_width / 4 * model_shared_image_height / 4) /// || (unsigned int)sb->cursize < (mipoffset[3] + model_shared_image_width / 8 * model_shared_image_height / 8 + 2 + 768) with pallete
	{
		TraceLog(LOG_WARNING, "model_shared.c: W_ConvertWAD3TextureBGRA: failed conditions, corrupted wad file");
		return NULL;
	}
	
	in = (unsigned char *)sb->data + mipoffset[0];
	data = out = (unsigned char *)Hunk_AllocNameNoFill(model_shared_image_width * model_shared_image_height * 4, "default hunk");
	if (!data)
		return NULL;
	for (d = 0;d < model_shared_image_width * model_shared_image_height;d++)
	{

		p = *in++;
		if (model_shared_texture_name[0] == '{' && p == 255)
			out[0] = out[1] = out[2] = out[3] = 0;
		else
		{
			p *= 3;
			out[0] = pal[p];
			out[1] = pal[p+1];
			out[2] = pal[p+2];
			out[3] = 255;
			


		}
		out += 4;
	}

	pixel_format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
	
	return data;
}

unsigned char *W_ConvertWAD3TextureRGBA(sizebuf_t *sb)
{
	unsigned char *in, *data, *out, *pal;
	int d, p;
	unsigned char name[16];
	unsigned int mipoffset[4];

	MSG_BeginReading(sb);
	MSG_ReadBytes(sb, 16, model_shared_texture_name);
	model_shared_texture_name[16] = 0;
	model_shared_image_width = MSG_ReadLittleLong(sb);
	model_shared_image_height = MSG_ReadLittleLong(sb);
	mipoffset[0] = MSG_ReadLittleLong(sb);
	mipoffset[1] = MSG_ReadLittleLong(sb); // should be mipoffset[0] + model_shared_image_width*model_shared_image_height
	mipoffset[2] = MSG_ReadLittleLong(sb); // should be mipoffset[1] + model_shared_image_width*model_shared_image_height/4
	mipoffset[3] = MSG_ReadLittleLong(sb); // should be mipoffset[2] + model_shared_image_width*model_shared_image_height/16
	pal = sb->data + mipoffset[3] + (model_shared_image_width / 8 * model_shared_image_height / 8) + 2;


	// bail if any data looks wrong
	if (model_shared_image_width < 0
	 || model_shared_image_width > 4096
	 || model_shared_image_height < 0
	 || model_shared_image_height > 4096
	 || mipoffset[0] != 40
	 || mipoffset[1] != mipoffset[0] + model_shared_image_width * model_shared_image_height
	 || mipoffset[2] != mipoffset[1] + model_shared_image_width / 2 * model_shared_image_height / 2
	 || mipoffset[3] != mipoffset[2] + model_shared_image_width / 4 * model_shared_image_height / 4
	 || (unsigned int)sb->cursize < (mipoffset[3] + model_shared_image_width / 8 * model_shared_image_height / 8 + 2 + 768))// with pallete
	{
		TraceLog(LOG_WARNING, "model_shared.c: W_ConvertWAD3TextureBGRA: failed conditions, corrupted wad file");
		return NULL;
	}
	
	in = (unsigned char *)sb->data + mipoffset[0];
	data = out = (unsigned char *)Hunk_AllocNameNoFill(model_shared_image_width * model_shared_image_height * 4, "default hunk");
	if (!data)
		return NULL;
	for (d = 0;d < model_shared_image_width * model_shared_image_height;d++)
	{

		p = *in++;
		if (model_shared_texture_name[0] == '{' && p == 255)
			out[0] = out[1] = out[2] = out[3] = 0;
		else
		{
			p *= 3;
			out[0] = pal[p];
			out[1] = pal[p+1];
			out[2] = pal[p+2];
			out[3] = 255;
		}
		out += 4;
	}

	pixel_format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
	
	return data;
}

unsigned char *W_ConvertWAD2TextureDXT1(sizebuf_t *sb)
{
	unsigned char *in, *data, *out, *pal;
	int d, p;
	unsigned char name[16];
	unsigned int mipoffset[4];

	MSG_BeginReading(sb);
	MSG_ReadBytes(sb, 16, model_shared_texture_name);
	model_shared_texture_name[16] = 0;
	model_shared_image_width = MSG_ReadLittleLong(sb);
	model_shared_image_height = MSG_ReadLittleLong(sb);
	mipoffset[0] = MSG_ReadLittleLong(sb);
	mipoffset[1] = MSG_ReadLittleLong(sb); // should be mipoffset[0] + model_shared_image_width*model_shared_image_height
	mipoffset[2] = MSG_ReadLittleLong(sb); // should be mipoffset[1] + model_shared_image_width*model_shared_image_height/4
	mipoffset[3] = MSG_ReadLittleLong(sb); // should be mipoffset[2] + model_shared_image_width*model_shared_image_height/16
	//pal = sb->data + mipoffset[3] + (model_shared_image_width / 8 * model_shared_image_height / 8) + 2;
	pal = (unsigned char*)Palette;

	// bail if any data looks wrong
	if (model_shared_image_width < 0
	 || model_shared_image_width > 4096
	 || model_shared_image_height < 0
	 || model_shared_image_height > 4096
	 || mipoffset[0] != 40
	 || mipoffset[1] != mipoffset[0] + model_shared_image_width * model_shared_image_height
	 || mipoffset[2] != mipoffset[1] + model_shared_image_width / 2 * model_shared_image_height / 2
	 || mipoffset[3] != mipoffset[2] + model_shared_image_width / 4 * model_shared_image_height / 4) /// || (unsigned int)sb->cursize < (mipoffset[3] + model_shared_image_width / 8 * model_shared_image_height / 8 + 2 + 768) with pallete
	{
		TraceLog(LOG_WARNING, "model_shared.c: W_ConvertWAD3TextureBGRA: failed conditions, corrupted wad file");
		return NULL;
	}
	int blockx = (model_shared_image_width+3)/4;
	int blocky = (model_shared_image_height+3)/4;
	in = (unsigned char *)sb->data + mipoffset[0];
	data = out = (unsigned char*)Hunk_AllocNoFill(blockx * blocky * 8);
	unsigned char src[4 * 4 * 4];

	int add_alpha = 0;
	

	if (model_shared_texture_name[0] == '{') {
		add_alpha = 1;
	}

	if (!data)
		return NULL;

	for (int by = 0; by < blocky; by++) {
		for (int bx = 0; bx < blockx; bx++) {


			for (int py = 0; py < 4; py++) {
				int pixel_y = by * 4 + py;
				if (pixel_y >= model_shared_image_height) pixel_y = model_shared_image_height-1;
				for (int px = 0; px < 4; px++) {
					int pixel_x = bx * 4 + px;
					if (pixel_x >= model_shared_image_width) pixel_x = model_shared_image_width-1;
					
					int rgba_index = (pixel_y*model_shared_image_width + pixel_x);
					int block_index = (py*4+px)*4;

					if (in[rgba_index] == 255) {
						src[block_index+0] = 0;
						src[block_index+1] = 0;
						src[block_index+2] = 0;
						src[block_index+3] = 0;
					} else {
						src[block_index+0] = pal[in[rgba_index]*3+0];
						src[block_index+1] = pal[in[rgba_index]*3+1];
						src[block_index+2] = pal[in[rgba_index]*3+2];
						src[block_index+3] = 255;
					}

					

				}
			}
			if (add_alpha) {
				stb_compress_dxt_block(out, src, 1, STB_DXT_NORMAL);
			} else {
				stb_compress_dxt_block(out, src, 0, STB_DXT_HIGHQUAL);
			}
				
			out += 8;

		}
	}

	pixel_format = PIXELFORMAT_COMPRESSED_DXT1_RGB;

	if (add_alpha) {
		pixel_format = PIXELFORMAT_COMPRESSED_DXT1_RGBA;
	}
	
	return data;
}

unsigned char *W_GetTextureBGRA(char *name)
{
	unsigned int i, k;
	sizebuf_t sb;
	unsigned char *data;
	mwad_t *w;
	char texname[17];
	size_t range;



	texname[16] = 0;
	W_CleanupName(name, texname);

	for (k = 0;k < wads.numwads;k++)
	{
		w = &(wads.w[k]);
		for (i = 0;i < (unsigned int)w->numlumps;i++)
		{
			if (!strcmp(texname, w->lumps[i].name)) // found it
			{
				
				if (fseek(w->file, w->lumps[i].filepos, SEEK_SET))
				{TraceLog(LOG_ERROR, "model_shared.c: W_GetTexture: corrupt WAD3 file\n");return NULL;}

				MSG_InitReadBuffer(&sb, (unsigned char *)Hunk_AllocNameNoFill(w->lumps[i].disksize, "default hunk"), w->lumps[i].disksize);
				if (!sb.data)
					return NULL;
				if (fread(sb.data, 1, w->lumps[i].size, w->file) < w->lumps[i].disksize)
				{TraceLog(LOG_ERROR, "model_shared.c: W_GetTexture: corrupt WAD3 file\n");return NULL;}

				if (w->version == 3)
					data = W_ConvertWAD3TextureRGBA(&sb);
				else 
					data = W_ConvertWAD2TextureRGBA(&sb);
				return data;
			}
		}
	}
	model_shared_image_width = model_shared_image_height = 0;
	return NULL;
}


void loadPalette(const char *fileName)
{


    if (fileName != NULL)
    {

        FILE *file = fopen(fileName, "rb");

        if (file != NULL)
        {
            // WARNING: On binary streams SEEK_END could not be found,
            // using fseek() and ftell() could not work in some (rare) cases
            fseek(file, 0, SEEK_END);
            int size = ftell(file);     // WARNING: ftell() returns 'long int', maximum size returned is INT_MAX (2147483647 bytes)
            fseek(file, 0, SEEK_SET);

            if (size == sizeof(Palette))
            {

                // NOTE: fread() returns number of read elements instead of bytes, so reading [1 byte, size elements]
                size_t count = fread(Palette, sizeof(unsigned char), size, file);

                // WARNING: fread() returns a size_t value, usually 'unsigned int' (32bit compilation) and 'unsigned long long' (64bit compilation)
                // dataSize is unified along raylib as a 'int' type, so, for file-sizes >INT_MAX (2147483647 bytes) there is a limitation
                if (count > 2147483647)
                {
                    TraceLog(LOG_WARNING, "model_shared.c: [%s] File is bigger than 2147483647 bytes, avoid using LoadFileData()", fileName);
                }
                else
                {

                    if ((int)count != size) TraceLog(LOG_WARNING, "model_shared.c: [%s] File partially loaded (%i bytes out of %i)", fileName, (int)count, size);
                    else TraceLog(LOG_INFO, "model_shared.c: [%s] File loaded successfully", fileName);
                }
            }
            else TraceLog(LOG_WARNING, "model_shared.c: [%s] Failed to read file", fileName);

            fclose(file);
            
        }
        else TraceLog(LOG_WARNING, "model_shared.c: [%s] Failed to open file", fileName);
    }
    else TraceLog(LOG_WARNING, "model_shared.c: File name provided is not valid");
}

#define MAX_KEY 32
#define MAX_VALUE 1024

typedef enum {
	PARSE_CLEAR,
	PARSE_ENTITY,
	PARSE_KEY,
	PARSE_ENTITY_VALUE,
	PARSE_VALUE,
} Parse_State;

void parseEntities(const char* str)
{
	int mark = Hunk_LowMark();
	int perEntityMark = mark;
	Parse_State state = PARSE_CLEAR;
	
	char key[MAX_KEY] = {0};
	char value[MAX_VALUE] = {0};

	int key_counter = 0;
	int value_counter = 0;

	int pairs = 0;

	for (; str[0]; str++) {
		switch (state)
		{
		case PARSE_CLEAR:
			if (str[0] == '{') {
				state = PARSE_ENTITY;
			}
			break;

		case PARSE_ENTITY:
			if (str[0] == '"') {
				state = PARSE_KEY;
				key_counter = 0;
				key[0] = '\0';
			}

			if (str[0] == '}') {
				G_CallSpawn();
				state = PARSE_CLEAR;
				pairs = 0;
				Hunk_FreeToLowMark(perEntityMark);
			}

			break;
		
		case PARSE_KEY:
			if (str[0] == '"') {
				state = PARSE_ENTITY_VALUE;
				key[key_counter++] = '\0';
			}
			else {
				key[key_counter++] = str[0];
				if (key_counter >= MAX_KEY) {
					key[MAX_KEY-1] = '\0';
					TraceLog(LOG_WARNING, "model_shared.c: parseEntities: key len out of range %i, (key: %s)", MAX_KEY, key);
					return;
				}
			}
			break;

		case PARSE_ENTITY_VALUE:
			if (str[0] == '"') {
				state = PARSE_VALUE;
				value_counter = 0;
				value[0] = '\0';
			}
			break;
		
		case PARSE_VALUE:
			if (str[0] == '"') {
				state = PARSE_ENTITY;
				value[value_counter++] = '\0';
				key[MAX_KEY-1] = '\0';
				value[MAX_VALUE-1] = '\0';
				spawnVars[pairs][0] = Hunk_AllocNoFill(key_counter);
				spawnVars[pairs][1] = Hunk_AllocNoFill(value_counter);
				memcpy_s(spawnVars[pairs][0], key_counter, key, key_counter);
				memcpy_s(spawnVars[pairs][1], value_counter, value, value_counter);
				pairs++;
				if (pairs >= MAX_SPAWN_VARS) {
					TraceLog(LOG_WARNING, "model_shared.c: spawn vars count out of range");
					pairs = 0;
					Hunk_FreeToLowMark(perEntityMark);
				}
				spawnVars[pairs][0] = NULL;
				spawnVars[pairs][1] = NULL;
				
				//TraceLog(LOG_INFO, "key [%s] value [%s]", key, value);
			}
			else {
				value[value_counter++] = str[0];
				if (value_counter >= MAX_VALUE) {
					key[MAX_KEY-1] = '\0';
					value[MAX_VALUE-1] = '\0';
					TraceLog(LOG_WARNING, "model_shared.c: parseEntities: value len out of range %i, (key: %s, value: %s)", MAX_VALUE, key, value);
					return;
				}
			}
			break;
		
		default:
			break;
		}
	}

	Hunk_FreeToLowMark(mark);
}

static void Mod_Q1BSP_LoadEntities(sizebuf_t *sb)
{
	loadmodel.entities = NULL;

	if (!sb->cursize)
		return;
	int beforemark = Hunk_LowMark();
	loadmodel.entities = (char *)Hunk_AllocNameNoFill(sb->cursize + 1, "entities");
	MSG_ReadBytes(sb, sb->cursize, (unsigned char *)loadmodel.entities);
	loadmodel.entities[sb->cursize] = 0;
	//if (loadmodel.ishlbsp) Mod_Q1BSP_ParseWadsFromEntityLump(loadmodel->brush.entities);
	
	//Hunk_FreeToLowMark(beforemark);
}

#define MY_MAX(a, b) (a > b ? a : b)
#define MY_MIN(a, b) (a < b ? a : b)

static void Mod_Q1BSP_LoadVertexes(sizebuf_t *sb)
{
	float* out;
	int			i, count;
	int			structsize = 12;

	if (sb->cursize % structsize)
		PRINT("Mod_Q1BSP_LoadVertexes: funny lump size in %s", loadmodel.name);
	count = sb->cursize / structsize;
	out = (float *)Hunk_AllocNameNoFill(count * 3 * sizeof(*out), "default hunk");

	loadmodel.vertices = out;
	loadmodel.vertexCount = count;

	loadmodel.aabb[0][0] = FLT_MAX;
	loadmodel.aabb[0][1] = FLT_MAX;
	loadmodel.aabb[0][2] = FLT_MAX;
	
	loadmodel.aabb[1][0] = -FLT_MAX;
	loadmodel.aabb[1][1] = -FLT_MAX;
	loadmodel.aabb[1][2] = -FLT_MAX;

	for ( i=0 ; i<count ; i++)
	{
		out[0] = -MSG_ReadLittleFloat(sb) * MULTIPLIER;
		out[2] = MSG_ReadLittleFloat(sb) * MULTIPLIER; // we should swap Y and Z axis for raylib, cause in quake Z points up
		out[1] = MSG_ReadLittleFloat(sb) * MULTIPLIER;

		loadmodel.aabb[0][0] = MY_MIN(loadmodel.aabb[0][0], out[0]);
		loadmodel.aabb[0][1] = MY_MIN(loadmodel.aabb[0][1], out[1]);
		loadmodel.aabb[0][2] = MY_MIN(loadmodel.aabb[0][2], out[2]);

		loadmodel.aabb[1][0] = MY_MAX(loadmodel.aabb[1][0], out[0]);
		loadmodel.aabb[1][1] = MY_MAX(loadmodel.aabb[1][1], out[1]);
		loadmodel.aabb[1][2] = MY_MAX(loadmodel.aabb[1][2], out[2]);

		out++;
		out++;
		out++;
	}   
}

static void Mod_Q1BSP_LoadEdges(sizebuf_t *sb)
{
	unsigned int* out;
	int 	i, count;
	int		structsize = loadmodel.isbsp2 ? 8 : 4;

	if (sb->cursize % structsize)
		PRINT("Mod_Q1BSP_LoadEdges: funny lump size %s", loadmodel.name);
	count = sb->cursize / structsize;
	out = (unsigned int*)Hunk_AllocNameNoFill(count * 2 * sizeof(*out), "default hunk");

	loadmodel.edges = out;
	loadmodel.numedges = count;

	for ( i=0 ; i<count*2 ; i++, out++)
	{
        if (loadmodel.isbsp2) {
            out[0] = (unsigned int)MSG_ReadLittleLong(sb);
        } else {
            out[0] = (unsigned short)MSG_ReadLittleShort(sb);
        }
		
		if ((int)out[0] >= loadmodel.vertexCount )
		{
			PRINT("Mod_Q1BSP_LoadEdges: in %s has invalid vertex indices in edge %i (vertices %i >= numvertices %i)", loadmodel.name, i, out[0], loadmodel.vertexCount);
			if(!loadmodel.vertexCount)
				PRINT("Mod_Q1BSP_LoadEdges: %s has edges but no vertexes, cannot fix", loadmodel.name);
				
			out[0] = 0;
		}
	}
}

static void Mod_Q1BSP_LoadSurfedges(sizebuf_t *sb)
{
	int		i;
	int structsize = 4;

	if (sb->cursize % structsize)
		PRINT("Mod_Q1BSP_LoadSurfedges: funny lump size in %s", loadmodel.name);
	loadmodel.numsurfedges = sb->cursize / structsize;
	loadmodel.surfedges = (int *)Hunk_AllocNameNoFill(loadmodel.numsurfedges * structsize, "default hunk");

	for (i = 0;i < loadmodel.numsurfedges;i++)
		loadmodel.surfedges[i] = MSG_ReadLittleLong(sb);
}



/*
================
Mod_TextureTypeFromName
================
*/
static textype_t Mod_TextureTypeFromName (const char *texname)
{
	if (texname[0] == '*')
	{
		if (!strncmp (texname + 1, "lava",  4))	return TEXTYPE_LAVA;
		if (!strncmp (texname + 1, "slime", 5))	return TEXTYPE_SLIME;
		if (!strncmp (texname + 1, "tele",  4))	return TEXTYPE_TELE;
		return TEXTYPE_WATER;
	}

	if (texname[0] == '{')
		return TEXTYPE_CUTOUT;

	if (!strncmp (texname,"sky",3) || !strncmp (texname,"pid",3))
		return TEXTYPE_SKY;

	return TEXTYPE_DEFAULT;
}

static void Mod_Q1BSP_LoadTextures(sizebuf_t *sb)
{
	int i, j, k, num, max, altmax, mtwidth, mtheight, doffset, incomplete, nummiptex = 0, firstskynoshadowtexture = 0;
	unsigned char *data, *mtdata;
	texture_t *tx;
	char name[MAX_QPATH];
	sizebuf_t miptexsb;

	texture_t temptexture = (texture_t){0};

	loadmodel.data_textures = NULL;

	// add two slots for notexture walls and notexture liquids, and duplicate
	// all sky textures; sky surfaces can be shadow-casting or not, the surface
	// loading will choose according to the contents behind the surface
	// (necessary to support e1m5 logo shadow which has a SKY contents brush,
	// while correctly treating sky textures as occluders in other situations).
	if (sb->cursize)
	{
		int numsky = 0;
		size_t watermark;
		nummiptex = MSG_ReadLittleLong(sb);
		loadmodel.num_textures = nummiptex + 2;	
		loadmodel.data_textures = (texture_t*)Hunk_AllocName(loadmodel.num_textures * sizeof(*loadmodel.data_textures), "data textures");
		// save the position so we can go back to it
		watermark = sb->readcount;
		for (i = 0; i < nummiptex; i++)
		{
			doffset = MSG_ReadLittleLong(sb);
			if (doffset == -1)
			{
				PRINT("%s: miptex #%i missing", loadmodel.name, i);
				continue;
			}

			MSG_InitReadBuffer(&miptexsb, sb->data + doffset, sb->cursize - doffset);

			// copy name, but only up to 16 characters
			// (the output buffer can hold more than this, but the input buffer is
			//  only 16)
			for (j = 0; j < 16; j++)
				name[j] = MSG_ReadByte(&miptexsb);
			name[j] = 0;
			// pretty up the buffer (replacing any trailing garbage with 0)
			for (j = (int)strlen(name); j < 16; j++)
				name[j] = 0;
			// bones_was_here: force all names to lowercase (matching code below) so we don't crash on e2m9
			for (j = 0;name[j];j++)
				if (name[j] >= 'A' && name[j] <= 'Z')
					name[j] += 'a' - 'A';
			mtwidth = MSG_ReadLittleLong(&miptexsb);
			mtheight = MSG_ReadLittleLong(&miptexsb);

			

			memcpy(temptexture.name, name, 16);
			temptexture.width = mtwidth;
			temptexture.height = mtheight;
			temptexture.type = Mod_TextureTypeFromName(temptexture.name);
			loadmodel.data_textures[i] = temptexture;

		}

		// bump it back to where we started parsing
		sb->readcount = (int)watermark;

		
	}
	else
	{
		loadmodel.num_textures = 0;
	}

	if (!sb->cursize)
	{
		return;
	}

	for (i = 0;i < nummiptex;i++)
	{
		doffset = MSG_ReadLittleLong(sb);

		if (doffset == -1)
		{
			TraceLog(LOG_WARNING, "model_shared.c: LoadTextures: %s: miptex #%i missing\n", loadmodel.name, i);
			continue;
		}
		
		if (loadmodel.data_textures[i].type == TEXTYPE_SKY) {
			continue;
		}

		MSG_InitReadBuffer(&miptexsb, sb->data + doffset, sb->cursize - doffset);
		int mark = Hunk_LowMark();
		if (loadmodel.ishlbsp)
			data = W_ConvertWAD3TextureRGBA(&miptexsb);
		else {
			data = W_ConvertWAD2TextureDXT1(&miptexsb);
			
		}
			

		if (data) {

			Image image = (Image){
				.data = data,
				.width = model_shared_image_width,
				.height = model_shared_image_height,
				.format = pixel_format,
				.mipmaps = 1
			};

			Texture texture = LoadTextureFromImage(image);
			GenTextureMipmaps(&texture);
			SetTextureFilter(texture, TEXTURE_FILTER_TRILINEAR);
			Resource_key key = setTextureResource(texture, model_shared_texture_name);

		}

		Hunk_FreeToLowMark(mark);

	}
}

static void Mod_Q1BSP_LoadTexinfo(sizebuf_t *sb)
{
	mtexinfo_t *out;
	int i, j, k, count, miptex;
	int structsize = 40;

	if (sb->cursize % structsize)
		PRINT("Mod_Q1BSP_LoadTexinfo: funny lump size in %s",loadmodel.name);
	count = sb->cursize / structsize;
	out = (mtexinfo_t *)Hunk_AllocNameNoFill( count * sizeof(*out), "default hunk");

	loadmodel.texinfo = out;
	loadmodel.numtexinfo = count;

	for (i = 0;i < count;i++, out++)
	{
		for (k = 0;k < 2;k++)
			for (j = 0;j < 4;j++)
				out->vecs[k][j] = MSG_ReadLittleFloat(sb);

		miptex = MSG_ReadLittleLong(sb);
		out->q1flags = MSG_ReadLittleLong(sb);

		if (out->q1flags & TEX_SPECIAL)
		{
			// if texture chosen is NULL or the shader needs a lightmap,
			// force to notexture water shader
			out->textureindex = loadmodel.num_textures - 1;
		}
		else
		{
			// if texture chosen is NULL, force to notexture
			out->textureindex = loadmodel.num_textures - 2;
		}
		// see if the specified miptex is valid and try to use it instead
		if (loadmodel.data_textures)
		{
			if ((unsigned int) miptex >= (unsigned int) loadmodel.num_textures)
				PRINT("error in model \"%s\": invalid miptex index %i(of %i)\n", loadmodel.name, miptex, loadmodel.num_textures);
			else
				out->textureindex = miptex;
		}

	}
}

static void Mod_Q1BSP_LoadLighting(sizebuf_t *sb)
{
	int i;
	unsigned char *in, *out, *data, d;
	char litfilename[MAX_QPATH];
	char dlitfilename[MAX_QPATH];
	int filesize;
	if (loadmodel.isbspx && loadmodel.lightdata)
	{
		return;
	}
	else if (loadmodel.ishlbsp) // LadyHavoc: load the colored lighting data straight
	{
		loadmodel.lightdata = (unsigned char *)Hunk_AllocNameNoFill(sb->cursize, "default hunk");
		loadmodel.num_lightdata = sb->cursize;
		for (i = 0;i < sb->cursize;i++)
			loadmodel.lightdata[i] = sb->data[i] >>= 1;
	}
	else // LadyHavoc: bsp version 29 (normal white lighting)
	{
		// LadyHavoc: hope is not lost yet, check for a .lit file to load
		strncpy(litfilename, loadmodel.name, MAX_QPATH);
		litfilename[MAX_QPATH-1] = 0;
		int len = strnlen(litfilename, MAX_QPATH);
		memcpy(&(litfilename[len-3]), "lit",3);
		data = (unsigned char*)LoadFileData(litfilename, &filesize);
		if (data)
		{
			if (filesize == (fs_offset_t)(8 + sb->cursize * 3) && data[0] == 'Q' && data[1] == 'L' && data[2] == 'I' && data[3] == 'T')
			{
				i = LittleLong(((int *)data)[1]);
				if (i == 1)	
				{
					TraceLog(LOG_INFO, "loaded %s\n", litfilename);
					loadmodel.lightdata = (unsigned char *)Hunk_AllocNameNoFill(filesize - 8, "default hunk");
					loadmodel.num_lightdata = filesize-8;
					memcpy(loadmodel.lightdata, data + 8, filesize - 8);
					free(data);
					return;
				}
				else
					TraceLog(LOG_INFO, "model_shared.c: Unknown .lit file version (%d)\n", i);
			}
			else if (filesize == 8)
				TraceLog(LOG_INFO, "model_shared.c: Empty .lit file, ignoring\n");
			else
				TraceLog(LOG_INFO, "model_shared.c: Corrupt .lit file (file size %i bytes, should be %i bytes), ignoring\n", (int) filesize, (int) (8 + sb->cursize * 3));
			if (data)
			{
				free(data);
				data = NULL;
			}
		}
		// LadyHavoc: oh well, expand the white lighting data
		if (!sb->cursize)
			return;
		loadmodel.lightdata = (unsigned char *)Hunk_AllocNameNoFill(sb->cursize*3, "default hunk");
		loadmodel.num_lightdata = sb->cursize*3;
		in = sb->data;
		out = loadmodel.lightdata;
		for (i = 0;i < sb->cursize;i++)
		{
			d = *in++;
			*out++ = d;
			*out++ = d;
			*out++ = d;
		}
	}
}

void PlaneClassify(mplane_t *p)
{
	// for optimized plane comparisons
	if (p->normal[0] == 1)
		p->type = 0;
	else if (p->normal[1] == 1)
		p->type = 1;
	else if (p->normal[2] == 1)
		p->type = 2;
	else
		p->type = 3;
	// for BoxOnPlaneSide
	p->signbits = 0;
	if (p->normal[0] < 0) // 1
		p->signbits |= 1;
	if (p->normal[1] < 0) // 2
		p->signbits |= 2;
	if (p->normal[2] < 0) // 4
		p->signbits |= 4;
}

static void Mod_Q1BSP_LoadPlanes(sizebuf_t *sb)
{
	int			i;
	mplane_t	*out;
	int structsize = 20;

	if (sb->cursize % structsize)
		TraceLog(LOG_ERROR, "model_shared.c: Mod_Q1BSP_LoadPlanes: funny lump size in %s", loadmodel.name);
	loadmodel.num_planes = sb->cursize / structsize;
	loadmodel.data_planes = out = (mplane_t *)Hunk_AllocNameNoFill(loadmodel.num_planes * sizeof(*out), "mplane_t");

	for (i = 0;i < loadmodel.num_planes;i++, out++)
	{
		out->normal[0] = MSG_ReadLittleFloat(sb);
		out->normal[1] = MSG_ReadLittleFloat(sb);
		out->normal[2] = MSG_ReadLittleFloat(sb);
		out->dist = MSG_ReadLittleFloat(sb);
		MSG_ReadLittleLong(sb); // type is not used, we use PlaneClassify
		PlaneClassify(out);
	}
}


static void putInAtlas(lightmap_state* lstate, int w, int h, int* latlasX, int* latlasY)
{
	
	if (lstate->my+h >= lstate->height) {
		*latlasX = 0;
		*latlasY = 0;
		return;
	}

	if (lstate->mx+w >= lstate->width) {
		lstate->mx = 0;
		lstate->my += lstate->curh;
		
		
		lstate->curh = h;
	} else {
		lstate->curh = (h > lstate->curh) ? h : lstate->curh;
	}

	
	*latlasX = lstate->mx;
	*latlasY = lstate->my;
	lstate->mx += w;

}

static void putToAtlasTexture(int w, int h, int x, int y, int lightmapoffset) 
{
	if (!loadmodel.lightTexture || !loadmodel.lightdata) {
		TraceLog(LOG_ERROR, "model_shared.c: failed to create light atlas"); 
		loadmodel.lightOverflow = true;
		return;
	}

	if ((x < 0) || (y < 0) || (x+w >= loadmodel.light_width) || (y+h >= loadmodel.light_height)) {
		TraceLog(LOG_ERROR, "model_shared.c: failed to create light atlas, out of data");
		loadmodel.lightOverflow = true;
		return;
	}

	int pixel = 0;

	

	for (int iy = y; iy < y+h; iy++) {
		for (int ix = x; ix < x+w; ix++) {
			pixel = iy * loadmodel.light_width + ix;
			if ((pixel+1) >= loadmodel.light_width*loadmodel.light_height || (lightmapoffset+1) >= loadmodel.num_lightdata) {
				TraceLog(LOG_ERROR, "model_shared.c: failed to create light atlas, something is off");
				loadmodel.lightOverflow = true;
				return;
			} else {
				loadmodel.lightTexture[pixel*3] = loadmodel.lightdata[lightmapoffset++];
				loadmodel.lightTexture[pixel*3+1] = loadmodel.lightdata[lightmapoffset++];
				loadmodel.lightTexture[pixel*3+2] = loadmodel.lightdata[lightmapoffset++];
			}
		}
	}
}

int compareAtlases(const void* a, const void* b)
{
	const atlase_t* at = *(const atlase_t**)a;
	const atlase_t* bt = *(const atlase_t**)b;

	return (bt->wy - at->wy);
}

static void Mod_Q1BSP_LoadFaces(sizebuf_t *sb)
{
    int i, j, count, surfacenum, planenum, smax, tmax, ssize, tsize, firstedge, numedges, totalverts, totaltris, lightmapnumber, lightmapsize, totallightmapsamples, lightmapoffset, texinfoindex, textureindex;
    int structsize = loadmodel.isbsp2 ? 28 : 20;
	unsigned short lmshift = 4;
    if (sb->cursize % structsize)
		PRINT("Mod_Q1BSP_LoadFaces: funny lump size in %s",loadmodel.name);
    count = sb->cursize / structsize;

	
	loadmodel.mesh = (mesh_t*)Hunk_AllocName(sizeof(*loadmodel.mesh), "mesh");
	loadmodel.meshCount = 1;

	loadmodel.num_surfaces = count;

	loadmodel.data_surfaces = (msurface_t *)Hunk_AllocName(loadmodel.num_surfaces*sizeof(msurface_t), "surfaces");

	//loadmodel.lmshifts = NULL;
	if (loadmodel.isbspx && loadmodel.lmshifts) {
		if (loadmodel.num_surfaces != loadmodel.num_lmshifts) {
			TraceLog(LOG_ERROR, "model_shared.c: Mod_Q1BSP_LoadFaces: num_surfaces %i and num_lmshifts %i dont match ", loadmodel.num_surfaces, loadmodel.num_lmshifts);
		}
	}
	

    totalverts = 0;
	totaltris = 0;
	for (surfacenum = 0;surfacenum < count;surfacenum++)
	{
		if (loadmodel.isbsp2) {
			numedges = BuffLittleLong(sb->data + structsize * surfacenum + 12);
			texinfoindex = BuffLittleLong(sb->data + structsize * surfacenum + 16);
		}
		else {
			numedges = BuffLittleShort(sb->data + structsize * surfacenum + 8);
			texinfoindex = BuffLittleShort(sb->data + structsize * surfacenum + 10);
		}

		if (texinfoindex >= loadmodel.numtexinfo)
			PRINT("Mod_Q1BSP_LoadFaces: invalid texinfo range (texinfo index %i, numtexinfo %i)", texinfoindex, loadmodel.numtexinfo);
		textureindex = loadmodel.texinfo[texinfoindex].textureindex;

		loadmodel.mesh[0].faceCount++;
			
		totalverts += numedges;
		totaltris += numedges - 2;
	}

	if (loadmodel.mesh[0].faceCount) {
		loadmodel.mesh[0].atlases = (atlase_t*)Hunk_AllocNameNoFill(loadmodel.mesh[0].faceCount*sizeof(atlase_t), "atlasesXY");
	} else {
		loadmodel.mesh[0].atlases = NULL;
	}

	
		
	sizebuf_t tempsb = *sb;


    
	loadmodel.indices = (int*)Hunk_AllocNameNoFill(totaltris*3*sizeof(int), "default hunk");
	loadmodel.triangleCount = totaltris;

	

    #define MAX_VERTICES_PER_FACE 64
    unsigned int verticesPerFace[MAX_VERTICES_PER_FACE];  
    float texcoordsPerFace[MAX_VERTICES_PER_FACE*2];
	float texcoords2PerFace[MAX_VERTICES_PER_FACE*2];

    totalverts = 0;
	totaltris = 0;

	
	
	int mark = Hunk_LowMark();

	atlase_t** atindexes = (atlase_t**)Hunk_AllocNameNoFill(count*sizeof(atlase_t*), "atlas_indexes");

	

    for (surfacenum = 0;surfacenum < count; surfacenum++)
	{

		// the struct on disk is the same in BSP29 (Q1), BSP30 (HL1), and IBSP38 (Q2)
		planenum = loadmodel.isbsp2 ? MSG_ReadLittleLong(sb) : (unsigned short)MSG_ReadLittleShort(sb);
		/*side = */loadmodel.isbsp2 ? MSG_ReadLittleLong(sb) : (unsigned short)MSG_ReadLittleShort(sb);
		firstedge = MSG_ReadLittleLong(sb);
		numedges = loadmodel.isbsp2 ? MSG_ReadLittleLong(sb) : (unsigned short)MSG_ReadLittleShort(sb);
		texinfoindex = loadmodel.isbsp2 ? MSG_ReadLittleLong(sb) : (unsigned short)MSG_ReadLittleShort(sb);
		for (i = 0;i < MAXLIGHTMAPS;i++)
			/*surface->lightmapinfo->styles[i] = */MSG_ReadByte(sb);
		if (loadmodel.ishlbsp) {
			lightmapoffset = MSG_ReadLittleLong(sb);
		} else {
			lightmapoffset = MSG_ReadLittleLong(sb)*3;
		}

		if (loadmodel.isbspx && loadmodel.lmshifts){
			lmshift = (unsigned short)loadmodel.lmshifts[surfacenum];
		}

		// FIXME: validate edges, texinfo, etc?
		if ((unsigned int) firstedge > (unsigned int) loadmodel.numsurfedges || (unsigned int) numedges > (unsigned int) loadmodel.numsurfedges || (unsigned int) firstedge + (unsigned int) numedges > (unsigned int) loadmodel.numsurfedges)
			PRINT("Mod_Q1BSP_LoadFaces: invalid edge range (firstedge %i, numedges %i, model edges %i)", firstedge, numedges, loadmodel.numsurfedges);
			if (texinfoindex >= loadmodel.numtexinfo)
			PRINT("Mod_Q1BSP_LoadFaces: invalid texinfo range (texinfo index %i, numtexinfo %i)", texinfoindex, loadmodel.numtexinfo);
		textureindex = loadmodel.texinfo[texinfoindex].textureindex;

		if (textureindex < 0 || textureindex >= loadmodel.num_textures) {
			TraceLog(LOG_WARNING, "model_shared.c: Mod_Q1BSP_LoadFaces: invalid texture index, setting it to zero");
			textureindex = 0;
		}

        int num_firstvertex = totalverts;
        int num_vertices = numedges;
        int num_firsttriangle = totaltris;
        int num_triangles = numedges - 2;

		loadmodel.mesh[0].triangleCount += num_triangles;
		loadmodel.mesh[0].vertexCount += num_vertices;

        totalverts += numedges;
		totaltris += numedges - 2;

        int surfedge;
        unsigned int edgeindex;

		if (numedges >= MAX_VERTICES_PER_FACE) {
			TraceLog(LOG_ERROR, "model_shared.c: Mod_Q1BSP_LoadFaces: Too much vertices per face %i (max %i)", numedges, MAX_VERTICES_PER_FACE);
		}

        for (i = 0; i < numedges; i++) {
            surfedge = loadmodel.surfedges[firstedge+i];
            if (surfedge > 0) {
                edgeindex = (unsigned int)surfedge;
                verticesPerFace[i] = loadmodel.edges[edgeindex*2+0];
            } else {
                edgeindex = (unsigned int)(-surfedge);
                verticesPerFace[i] = loadmodel.edges[edgeindex*2+1];
            }
                
        }


		int triangleIndex = 0;

        for (i = 0;i < num_triangles;i++)
		{
			triangleIndex = num_firsttriangle+i;

			loadmodel.indices[triangleIndex*3+0] = verticesPerFace[0];
			loadmodel.indices[triangleIndex*3+2] = verticesPerFace[i+1];
			loadmodel.indices[triangleIndex*3+1] = verticesPerFace[i+2];
		}



		vec3 u_normal = (vec3){
			loadmodel.texinfo[texinfoindex].vecs[0][0],
			loadmodel.texinfo[texinfoindex].vecs[0][1],
			loadmodel.texinfo[texinfoindex].vecs[0][2]
		};

		float u_offset = loadmodel.texinfo[texinfoindex].vecs[0][3];

		vec3 v_normal = (vec3){
			loadmodel.texinfo[texinfoindex].vecs[1][0],
			loadmodel.texinfo[texinfoindex].vecs[1][1],
			loadmodel.texinfo[texinfoindex].vecs[1][2]
		};

		float v_offset = loadmodel.texinfo[texinfoindex].vecs[1][3];

		float ucoord = 0.0f;
		float vcoord = 0.0f;

		float max_ucoord = -FLT_MAX;
		float max_vcoord = -FLT_MAX;

		float min_ucoord = FLT_MAX;
		float min_vcoord = FLT_MAX;

		vec3 gp = (vec3){0};
		for (i = 0; i < numedges; i++) {
            gp.x = loadmodel.vertices[verticesPerFace[i]*3];
			gp.y = loadmodel.vertices[verticesPerFace[i]*3+1];
			gp.z = loadmodel.vertices[verticesPerFace[i]*3+2];

			ucoord = (dot_product(swapyz(gp), u_normal)*INV_MULTI + u_offset);
			vcoord = (dot_product(swapyz(gp), v_normal)*INV_MULTI + v_offset);

			texcoordsPerFace[i*2] = ucoord;
			texcoordsPerFace[i*2+1] = vcoord;

			max_ucoord = (ucoord > max_ucoord) ? ucoord : max_ucoord;
			max_vcoord = (vcoord > max_vcoord) ? vcoord : max_vcoord;
			min_ucoord = (ucoord > min_ucoord) ? min_ucoord : ucoord;
			min_vcoord = (vcoord > min_vcoord) ? min_vcoord : vcoord;
                
        }

		int min_u_16 = (int)floorf(min_ucoord / (1<<lmshift));
		int max_u_16 = (int)ceilf(max_ucoord / (1<<lmshift));
		int min_v_16 = (int)floorf(min_vcoord / (1<<lmshift));
		int max_v_16 = (int)ceilf(max_vcoord / (1<<lmshift));

		int lm_w = (max_u_16 - min_u_16) + 1;
		int lm_h = (max_v_16 - min_v_16) + 1;


		int latlasX = 0;
		int latlasY = 0;

		if (lightmapoffset >= 0){
			loadmodel.mesh[0].atlases[loadmodel.mesh[0].num_firstface].wx = lm_w;
			loadmodel.mesh[0].atlases[loadmodel.mesh[0].num_firstface].wy = lm_h;
		} else {
			loadmodel.mesh[0].atlases[loadmodel.mesh[0].num_firstface].wx = 0;
			loadmodel.mesh[0].atlases[loadmodel.mesh[0].num_firstface].wy = 0;
		}

		loadmodel.mesh[0].atlases[loadmodel.mesh[0].num_firstface].offset = lightmapoffset;

		atindexes[surfacenum] = &(loadmodel.mesh[0].atlases[loadmodel.mesh[0].num_firstface]);

		loadmodel.mesh[0].num_firstface++;

    }

	*sb = tempsb; //restore pointers

	qsort(atindexes, count, sizeof(atlase_t*), compareAtlases);

	int power_of_two = 16;
	int power_of_two_twiced = power_of_two * power_of_two;
	int power = 4;

	for (int penistuy = 0; penistuy < 20; penistuy++) {
		power_of_two <<= 1;
		power_of_two_twiced <<= 2;
		if (power_of_two_twiced > loadmodel.num_lightdata/3) {
			break;
		}
	}
	
	loadmodel.light_width = power_of_two;
	loadmodel.light_height = power_of_two*2;
	loadmodel.lightTexture = (unsigned char*)Hunk_AllocName(loadmodel.light_width*loadmodel.light_height*3, "lightatlas");

	lightmap_state lstate = (lightmap_state){.curh = 0, .mx = 0, .my = 0, .width = loadmodel.light_width, .height = loadmodel.light_height};

	
	atlase_t* temp = NULL;
	int latlasX = 0;
	int latlasY = 0;
	putInAtlas(&lstate, 10, 10, &(latlasX), &(latlasY));
	for (surfacenum = 0; surfacenum < count; surfacenum++) {
		temp = atindexes[surfacenum];
		if (temp->offset >= 0){
			putInAtlas(&lstate, temp->wx, temp->wy, &(temp->ax), &(temp->ay));
			putToAtlasTexture(temp->wx, temp->wy, temp->ax, temp->ay, temp->offset);
		} else {
			temp->ax = latlasX;
			temp->ay = latlasY;
			temp->wx = 10;
			temp->wy = 10;
		}
			
	}

	Image image = (Image){
		.data = loadmodel.lightTexture,
		.width = loadmodel.light_width,
		.height = lstate.my + lstate.curh,
		.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8,
		.mipmaps = 1
	};


	Texture lightmapVRAM = LoadTextureFromImage(image);
	//GenTextureMipmaps(&lightmapVRAM);
	SetTextureFilter(lightmapVRAM, TEXTURE_FILTER_TRILINEAR);
	setTextureResource(lightmapVRAM, "\\light");

	Hunk_FreeToLowMark(mark);
	atindexes = NULL;
	loadmodel.lightTexture = NULL;
	loadmodel.light_width = 0;
	loadmodel.light_height = 0;
	

	
	//loadmodel.lightOverflow = true;

	loadmodel.mesh[0].num_firstface = 0;
	if (loadmodel.mesh[0].triangleCount) {
		loadmodel.mesh[0].vertices = (float*)Hunk_AllocNameNoFill(loadmodel.mesh[0].vertexCount*3*4, "vertices");
		loadmodel.mesh[0].normals = (float*)Hunk_AllocNameNoFill(loadmodel.mesh[0].vertexCount*3*4, "normals");
		loadmodel.mesh[0].texcoords = (float*)Hunk_AllocNameNoFill(loadmodel.mesh[0].vertexCount*2*4, "texcoords");
		loadmodel.mesh[0].texcoords2 = (float*)Hunk_AllocNameNoFill(loadmodel.mesh[0].vertexCount*2*4, "texcoords2");
	}

	

	
    totalverts = 0;
	totaltris = 0;

	int texWidth = 0;
	int texHeight = 0;
	float inv_texWidth = 0.0f;
	float inv_texHeight = 0.0f;

	msurface_t *surface;

    for (surfacenum = 0, surface = loadmodel.data_surfaces;surfacenum < count; surfacenum++, surface++)
	{

		// the struct on disk is the same in BSP29 (Q1), BSP30 (HL1), and IBSP38 (Q2)
		planenum = loadmodel.isbsp2 ? MSG_ReadLittleLong(sb) : (unsigned short)MSG_ReadLittleShort(sb);
		/*side = */loadmodel.isbsp2 ? MSG_ReadLittleLong(sb) : (unsigned short)MSG_ReadLittleShort(sb);
		firstedge = MSG_ReadLittleLong(sb);
		numedges = loadmodel.isbsp2 ? MSG_ReadLittleLong(sb) : (unsigned short)MSG_ReadLittleShort(sb);
		texinfoindex = loadmodel.isbsp2 ? MSG_ReadLittleLong(sb) : (unsigned short)MSG_ReadLittleShort(sb);
		for (i = 0;i < MAXLIGHTMAPS;i++)
			/*surface->lightmapinfo->styles[i] = */MSG_ReadByte(sb);
		if (loadmodel.ishlbsp) {
			lightmapoffset = MSG_ReadLittleLong(sb);
		} else {
			lightmapoffset = MSG_ReadLittleLong(sb)*3;
		}

		if (loadmodel.isbspx && loadmodel.lmshifts){
			lmshift = (unsigned short)loadmodel.lmshifts[surfacenum];
		}


		// FIXME: validate edges, texinfo, etc?
		if ((unsigned int) firstedge > (unsigned int) loadmodel.numsurfedges || (unsigned int) numedges > (unsigned int) loadmodel.numsurfedges || (unsigned int) firstedge + (unsigned int) numedges > (unsigned int) loadmodel.numsurfedges)
			PRINT("Mod_Q1BSP_LoadFaces: invalid edge range (firstedge %i, numedges %i, model edges %i)", firstedge, numedges, loadmodel.numsurfedges);
		if (texinfoindex >= loadmodel.numtexinfo)
			PRINT("Mod_Q1BSP_LoadFaces: invalid texinfo range (texinfo index %i, numtexinfo %i)", texinfoindex, loadmodel.numtexinfo);
		textureindex = loadmodel.texinfo[texinfoindex].textureindex;
		texWidth = loadmodel.data_textures[textureindex].width;
		texHeight = loadmodel.data_textures[textureindex].height;

		if (texWidth && texHeight) {
			inv_texWidth = 1.0f/(float)texWidth;
			inv_texHeight = 1.0f/(float)texHeight;
		}
			

        int num_firstvertex = totalverts;
        int num_vertices = numedges;
        int num_firsttriangle = totaltris;
        int num_triangles = numedges - 2;

        totalverts += numedges;
		totaltris += numedges - 2;

		

        int surfedge;
        unsigned int edgeindex;
		

        for (i = 0; i < numedges; i++) {
            surfedge = loadmodel.surfedges[firstedge+i];
            if (surfedge > 0) {
                edgeindex = (unsigned int)surfedge;
                verticesPerFace[i] = loadmodel.edges[edgeindex*2+0];
            } else {
                edgeindex = (unsigned int)(-surfedge);
                verticesPerFace[i] = loadmodel.edges[edgeindex*2+1];
            }
                
        }


		vec3 u_normal = (vec3){
			loadmodel.texinfo[texinfoindex].vecs[0][0],
			loadmodel.texinfo[texinfoindex].vecs[0][1],
			loadmodel.texinfo[texinfoindex].vecs[0][2]
		};

		float u_offset = loadmodel.texinfo[texinfoindex].vecs[0][3];

		vec3 v_normal = (vec3){
			loadmodel.texinfo[texinfoindex].vecs[1][0],
			loadmodel.texinfo[texinfoindex].vecs[1][1],
			loadmodel.texinfo[texinfoindex].vecs[1][2]
		};

		float v_offset = loadmodel.texinfo[texinfoindex].vecs[1][3];

		float ucoord = 0.0f;
		float vcoord = 0.0f;

		float max_ucoord = -FLT_MAX;
		float max_vcoord = -FLT_MAX;

		float min_ucoord = FLT_MAX;
		float min_vcoord = FLT_MAX;


		vec3 gp = (vec3){0};
		for (i = 0; i < numedges; i++) {
            gp.x = loadmodel.vertices[verticesPerFace[i]*3];
			gp.y = loadmodel.vertices[verticesPerFace[i]*3+1];
			gp.z = loadmodel.vertices[verticesPerFace[i]*3+2];

			ucoord = (dot_product(swapyz(gp), u_normal)*INV_MULTI + u_offset);
			vcoord = (dot_product(swapyz(gp), v_normal)*INV_MULTI + v_offset);

			texcoordsPerFace[i*2] = ucoord;
			texcoordsPerFace[i*2+1] = vcoord;

			max_ucoord = (ucoord > max_ucoord) ? ucoord : max_ucoord;
			max_vcoord = (vcoord > max_vcoord) ? vcoord : max_vcoord;
			min_ucoord = (ucoord > min_ucoord) ? min_ucoord : ucoord;
			min_vcoord = (vcoord > min_vcoord) ? min_vcoord : vcoord;
                
        }

		for (i = 0; i < numedges; i++) {

			texcoords2PerFace[i*2] = texcoordsPerFace[i*2] - min_ucoord;
			texcoords2PerFace[i*2+1] = texcoordsPerFace[i*2+1] - min_vcoord;

			texcoordsPerFace[i*2] *= inv_texWidth;
			texcoordsPerFace[i*2+1] *= inv_texHeight;
                
        }
		

		int min_u_16 = (int)floorf(min_ucoord / (1<<lmshift));
		int max_u_16 = (int)ceilf(max_ucoord / (1<<lmshift));
		int min_v_16 = (int)floorf(min_vcoord / (1<<lmshift));
		int max_v_16 = (int)ceilf(max_vcoord / (1<<lmshift));

		int lm_w = (max_u_16 - min_u_16) + 1;
		int lm_h = (max_v_16 - min_v_16) + 1;


		int latlasX = 0;
		int latlasY = 0;

		latlasX = loadmodel.mesh[0].atlases[loadmodel.mesh[0].num_firstface].ax;
		latlasY = loadmodel.mesh[0].atlases[loadmodel.mesh[0].num_firstface].ay;



		

		loadmodel.mesh[0].num_firstface++;

		vec3 p1 = (vec3){	
			loadmodel.vertices[verticesPerFace[0]*3],
			loadmodel.vertices[verticesPerFace[0]*3+1],
			loadmodel.vertices[verticesPerFace[0]*3+2]
		};

		vec3 p3 = (vec3){	
			loadmodel.vertices[verticesPerFace[1]*3],
			loadmodel.vertices[verticesPerFace[1]*3+1],
			loadmodel.vertices[verticesPerFace[1]*3+2]
		};

		vec3 p2 = (vec3){	
			loadmodel.vertices[verticesPerFace[2]*3],
			loadmodel.vertices[verticesPerFace[2]*3+1],
			loadmodel.vertices[verticesPerFace[2]*3+2]
		};

		vec3 normal = calc_normal(p1, p2, p3);

		

		surface->tex_idx = textureindex;
		surface->num_firsttriangle = loadmodel.mesh[0].num_firsttriangle;
		surface->num_firstvertex = loadmodel.mesh[0].num_firstvertex;
		surface->num_triangles = num_triangles;
		surface->num_vertices = num_vertices;
		surface->included = false;


		int idx = 0;

		

        for (i = 0;i < num_vertices;i++)
		{

			idx = loadmodel.mesh[0].num_firstvertex;

			p1.x = loadmodel.vertices[verticesPerFace[i]*3];
			p1.y = loadmodel.vertices[verticesPerFace[i]*3+1];
			p1.z = loadmodel.vertices[verticesPerFace[i]*3+2];


            loadmodel.mesh[0].vertices[idx*3] = p1.x;
            loadmodel.mesh[0].vertices[idx*3+1] = p1.y;
            loadmodel.mesh[0].vertices[idx*3+2] = p1.z;

			loadmodel.mesh[0].normals[idx*3] = normal.x;
			loadmodel.mesh[0].normals[idx*3+1] = normal.y;
			loadmodel.mesh[0].normals[idx*3+2] = normal.z;

			loadmodel.mesh[0].texcoords[idx*2+0] = texcoordsPerFace[i*2];
			loadmodel.mesh[0].texcoords[idx*2+1] = texcoordsPerFace[i*2+1];

			if (lightmapoffset >= 0) {
				loadmodel.mesh[0].texcoords2[idx*2+0] = (latlasX + texcoords2PerFace[i*2] / (1<<lmshift) + 0.5f) / (float)lstate.width;
				loadmodel.mesh[0].texcoords2[idx*2+1] = (latlasY + texcoords2PerFace[i*2+1] / (1<<lmshift) + 0.5f) / (float)(lstate.my + lstate.curh);
			} else {
				loadmodel.mesh[0].texcoords2[idx*2+0] = (latlasX) / (float)lstate.width;
				loadmodel.mesh[0].texcoords2[idx*2+1] = (latlasY) / (float)(lstate.my + lstate.curh);
			}

			idx++;
			

			loadmodel.mesh[0].num_firstvertex = idx;
			


		}

		loadmodel.mesh[0].num_firsttriangle += num_triangles;

    }



}

static void Mod_Q1BSP_LoadLeaffaces(sizebuf_t *sb)
{
	int i, j;
	int structsize = loadmodel.isbsp2 ? 4 : 2;

	if (sb->cursize % structsize)
		TraceLog(LOG_ERROR, "model_shared.c: Mod_Q1BSP_LoadLeaffaces: funny lump size in %s",loadmodel.name);
	loadmodel.num_leafsurfaces = sb->cursize / structsize;
	loadmodel.data_leafsurfaces = (int *)Hunk_AllocNameNoFill(loadmodel.num_leafsurfaces * sizeof(int), "leaf faces");

	if (loadmodel.isbsp2)
	{
		for (i = 0;i < loadmodel.num_leafsurfaces;i++)
		{
			j = MSG_ReadLittleLong(sb);
			if (j < 0 || j >= loadmodel.num_surfaces)
				TraceLog(LOG_ERROR, "model_shared.c: Mod_Q1BSP_LoadLeaffaces: bad surface number");
			loadmodel.data_leafsurfaces[i] = j;
		}
	}
	else
	{
		for (i = 0;i < loadmodel.num_leafsurfaces;i++)
		{
			j = (unsigned short) MSG_ReadLittleShort(sb);
			if (j >= loadmodel.num_surfaces)
				TraceLog(LOG_ERROR, "model_shared.c: Mod_Q1BSP_LoadLeaffaces: bad surface number");
			loadmodel.data_leafsurfaces[i] = j;
		}
	}
}

static void Mod_Q1BSP_LoadVisibility(sizebuf_t *sb)
{
	loadmodel.num_compressedpvs = 0;
	loadmodel.data_compressedpvs = NULL;
	if (!sb->cursize)
		return;
	loadmodel.num_compressedpvs = sb->cursize;
	loadmodel.data_compressedpvs = (unsigned char *)Hunk_AllocNameNoFill(sb->cursize, "visibility");
	MSG_ReadBytes(sb, sb->cursize, loadmodel.data_compressedpvs);
}

static void Mod_BSP_LoadSubmodels(sizebuf_t *sb, hullinfo_t *hullinfo)
{
	mmodel_t	*out;
	int			i, j, count;
	int			structsize = hullinfo ? (48+4*hullinfo->filehulls) : 48;

	if (sb->cursize % structsize)
		TraceLog(LOG_ERROR, "model_shared.c: Mod_BSP_LoadSubmodels: funny lump size in %s", loadmodel.name);

	count = sb->cursize / structsize;
	out = (mmodel_t *)Hunk_Alloc(count*sizeof(*out));

	loadmodel.submodels = out;
	loadmodel.numsubmodels = count;

	for (i = 0; i < count; i++, out++)
	{
	// spread out the mins / maxs by a pixel
		out->mins[0] = MSG_ReadLittleFloat(sb) - 1;
		out->mins[1] = MSG_ReadLittleFloat(sb) - 1;
		out->mins[2] = MSG_ReadLittleFloat(sb) - 1;
		out->maxs[0] = MSG_ReadLittleFloat(sb) + 1;
		out->maxs[1] = MSG_ReadLittleFloat(sb) + 1;
		out->maxs[2] = MSG_ReadLittleFloat(sb) + 1;
		out->origin[0] = MSG_ReadLittleFloat(sb);
		out->origin[1] = MSG_ReadLittleFloat(sb);
		out->origin[2] = MSG_ReadLittleFloat(sb);
		if(hullinfo)
		{
			for (j = 0; j < hullinfo->filehulls; j++)
				out->headnode[j] = MSG_ReadLittleLong(sb);
			out->visleafs  = MSG_ReadLittleLong(sb);
		}
		else // Quake 2 has only one hull
			out->headnode[0] = MSG_ReadLittleLong(sb);

		out->firstface = MSG_ReadLittleLong(sb);
		out->numfaces  = MSG_ReadLittleLong(sb);
	}
}

static void Mod_Q1BSP_LoadLeafs(sizebuf_t *sb)
{
	mleaf_t *out;
	int i, j, count, p, firstmarksurface, nummarksurfaces;
	int structsize = loadmodel.isbsp2rmqe ? 32 : (loadmodel.isbsp2 ? 44 : 28);

	if (sb->cursize % structsize)
		TraceLog(LOG_ERROR, "model_shared.c: Mod_Q1BSP_LoadLeafs: funny lump size in %s",loadmodel.name);
	count = sb->cursize / structsize;
	out = (mleaf_t *)Hunk_AllocName(count*sizeof(*out), "leafs");

	loadmodel.data_leafs = out;
	loadmodel.num_leafs = count;


	// FIXME: this function could really benefit from some error checking
	for ( i=0 ; i<count ; i++, out++)
	{
		out->contents = MSG_ReadLittleLong(sb);
		out->combinedsupercontents = out->contents;

		p = MSG_ReadLittleLong(sb);
		out->clusterindex = p; 

		if (loadmodel.isbsp2rmqe)
		{
			out->mins[0] = MSG_ReadLittleShort(sb);
			out->mins[1] = MSG_ReadLittleShort(sb);
			out->mins[2] = MSG_ReadLittleShort(sb);
			out->maxs[0] = MSG_ReadLittleShort(sb);
			out->maxs[1] = MSG_ReadLittleShort(sb);
			out->maxs[2] = MSG_ReadLittleShort(sb);
	
			firstmarksurface = MSG_ReadLittleLong(sb);
			nummarksurfaces = MSG_ReadLittleLong(sb);
		}
		else if (loadmodel.isbsp2)
		{
			out->mins[0] = MSG_ReadLittleFloat(sb);
			out->mins[1] = MSG_ReadLittleFloat(sb);
			out->mins[2] = MSG_ReadLittleFloat(sb);
			out->maxs[0] = MSG_ReadLittleFloat(sb);
			out->maxs[1] = MSG_ReadLittleFloat(sb);
			out->maxs[2] = MSG_ReadLittleFloat(sb);
	
			firstmarksurface = MSG_ReadLittleLong(sb);
			nummarksurfaces = MSG_ReadLittleLong(sb);
		}
		else
		{
			out->mins[0] = MSG_ReadLittleShort(sb);
			out->mins[1] = MSG_ReadLittleShort(sb);
			out->mins[2] = MSG_ReadLittleShort(sb);
			out->maxs[0] = MSG_ReadLittleShort(sb);
			out->maxs[1] = MSG_ReadLittleShort(sb);
			out->maxs[2] = MSG_ReadLittleShort(sb);
	
			firstmarksurface = (unsigned short)MSG_ReadLittleShort(sb);
			nummarksurfaces  = (unsigned short)MSG_ReadLittleShort(sb);
		}

		if (firstmarksurface >= 0 && firstmarksurface + nummarksurfaces <= loadmodel.num_leafsurfaces)
		{
			out->firstleafsurface = loadmodel.data_leafsurfaces + firstmarksurface;
			out->numleafsurfaces = nummarksurfaces;
		}
		else
		{
			TraceLog(LOG_ERROR, "model_shared.c: Mod_Q1BSP_LoadLeafs: invalid leafsurface range %i:%i outside range %i:%i\n", firstmarksurface, firstmarksurface+nummarksurfaces, 0, loadmodel.num_leafsurfaces);
			out->firstleafsurface = NULL;
			out->numleafsurfaces = 0;
		}

		for (j = 0;j < 4;j++)
			out->ambient_sound_level[j] = MSG_ReadByte(sb);
	}
}

static void Mod_Q1BSP_LoadNodes(sizebuf_t *sb)
{
	int			i, j, count, p, child[2];
	mnode_t 	*out;
	int structsize = loadmodel.isbsp2rmqe ? 32 : (loadmodel.isbsp2 ? 44 : 24);

	if (sb->cursize % structsize)
		TraceLog(LOG_ERROR, "model_shared.c: Mod_Q1BSP_LoadNodes: funny lump size in %s",loadmodel.name);
	count = sb->cursize / structsize;
	if (count == 0)
		TraceLog(LOG_ERROR, "model_shared.c: Mod_Q1BSP_LoadNodes: missing BSP tree in %s",loadmodel.name);
	out = (mnode_t *)Hunk_AllocName(count*sizeof(*out), "nodes");

	loadmodel.data_nodes = out;
	loadmodel.num_nodes = count;

	for ( i=0 ; i<count ; i++, out++)
	{
		p = MSG_ReadLittleLong(sb);
		out->plane = loadmodel.data_planes + p;

		if (loadmodel.isbsp2rmqe)
		{
			child[0] = MSG_ReadLittleLong(sb);
			child[1] = MSG_ReadLittleLong(sb);
			out->mins[0] = MSG_ReadLittleShort(sb);
			out->mins[1] = MSG_ReadLittleShort(sb);
			out->mins[2] = MSG_ReadLittleShort(sb);
			out->maxs[0] = MSG_ReadLittleShort(sb);
			out->maxs[1] = MSG_ReadLittleShort(sb);
			out->maxs[2] = MSG_ReadLittleShort(sb);
			out->firstsurface = MSG_ReadLittleLong(sb);
			out->numsurfaces = MSG_ReadLittleLong(sb);
		}
		else if (loadmodel.isbsp2)
		{
			child[0] = MSG_ReadLittleLong(sb);
			child[1] = MSG_ReadLittleLong(sb);
			out->mins[0] = MSG_ReadLittleFloat(sb);
			out->mins[1] = MSG_ReadLittleFloat(sb);
			out->mins[2] = MSG_ReadLittleFloat(sb);
			out->maxs[0] = MSG_ReadLittleFloat(sb);
			out->maxs[1] = MSG_ReadLittleFloat(sb);
			out->maxs[2] = MSG_ReadLittleFloat(sb);
			out->firstsurface = MSG_ReadLittleLong(sb);
			out->numsurfaces = MSG_ReadLittleLong(sb);
		}
		else
		{
			child[0] = (unsigned short)MSG_ReadLittleShort(sb);
			child[1] = (unsigned short)MSG_ReadLittleShort(sb);
			if (child[0] >= count)
				child[0] -= 65536;
			if (child[1] >= count)
				child[1] -= 65536;

			out->mins[0] = MSG_ReadLittleShort(sb);
			out->mins[1] = MSG_ReadLittleShort(sb);
			out->mins[2] = MSG_ReadLittleShort(sb);
			out->maxs[0] = MSG_ReadLittleShort(sb);
			out->maxs[1] = MSG_ReadLittleShort(sb);
			out->maxs[2] = MSG_ReadLittleShort(sb);

			out->firstsurface = (unsigned short)MSG_ReadLittleShort(sb);
			out->numsurfaces = (unsigned short)MSG_ReadLittleShort(sb);
		}

		for (j=0 ; j<2 ; j++)
		{
			// LadyHavoc: this code supports broken bsp files produced by
			// arguire qbsp which can produce more than 32768 nodes, any value
			// below count is assumed to be a node number, any other value is
			// assumed to be a leaf number
			p = child[j];
			if (p >= 0)
			{
				if (p < loadmodel.num_nodes)
					out->children[j] = loadmodel.data_nodes + p;
				else
				{
					TraceLog(LOG_WARNING, "model-shared.c: Mod_Q1BSP_LoadNodes: invalid node index %i (file has only %i nodes)\n", p, loadmodel.num_nodes);
					// map it to the solid leaf
					out->children[j] = (mnode_t *)loadmodel.data_leafs;
				}
			}
			else
			{
				// get leaf index as a positive value starting at 0 (-1 becomes 0, -2 becomes 1, etc)
				p = -(p+1);
				if (p < loadmodel.num_leafs)
					out->children[j] = (mnode_t *)(loadmodel.data_leafs + p);
				else
				{
					TraceLog(LOG_ERROR, "model_shared.c: Mod_Q1BSP_LoadNodes: invalid leaf index %i (file has only %i leafs)\n", p, loadmodel.num_leafs);
					// map it to the solid leaf
					out->children[j] = (mnode_t *)loadmodel.data_leafs;
				}
			}
		}
	}

	//Mod_BSP_LoadNodes_RecursiveSetParent(loadmodel.data_nodes, NULL);	// sets nodes and leafs
}


#define VectorSet(vec,x,y,z) ((vec)[0]=(x),(vec)[1]=(y),(vec)[2]=(z))
#define VectorClear(a) ((a)[0]=(a)[1]=(a)[2]=0)



void loadBSP(model_t* mod, void* data, void* dataEnd)
{
    int i, j, k;
    sizebuf_t lumpsb[HEADER_LUMPS];
    sizebuf_t sb;
	hullinfo_t hullinfo;

    memcpy(&loadmodel, mod, sizeof(*mod));

	


    MSG_InitReadBuffer(&sb, (unsigned char *)data, (unsigned char *)dataEnd - (unsigned char *)data);

    i = MSG_ReadLittleLong(&sb);

	VectorClear (hullinfo.hullsizes[0][0]);
	VectorClear (hullinfo.hullsizes[0][1]);
	if (loadmodel.ishlbsp)
	{
		hullinfo.filehulls = 4;
		VectorSet (hullinfo.hullsizes[1][0], -16, -16, -36);
		VectorSet (hullinfo.hullsizes[1][1], 16, 16, 36);
		VectorSet (hullinfo.hullsizes[2][0], -32, -32, -32);
		VectorSet (hullinfo.hullsizes[2][1], 32, 32, 32);
		VectorSet (hullinfo.hullsizes[3][0], -16, -16, -18);
		VectorSet (hullinfo.hullsizes[3][1], 16, 16, 18);
	}
	else
	{
		hullinfo.filehulls = 4;
		VectorSet (hullinfo.hullsizes[1][0], -16, -16, -24);
		VectorSet (hullinfo.hullsizes[1][1], 16, 16, 32);
		VectorSet (hullinfo.hullsizes[2][0], -32, -32, -24);
		VectorSet (hullinfo.hullsizes[2][1], 32, 32, 64);
	}

	size_t maxLumpEnd = 0;

    for (i = 0; i < HEADER_LUMPS; i++) {
        int offset = MSG_ReadLittleLong(&sb);
        int size = MSG_ReadLittleLong(&sb);
		int lumpEnd = offset + size;
        if (offset < 0 || lumpEnd > sb.cursize)
			PRINT("loadBSP: has invalid lump %i (offset %i, size %i, file size %i)", i, offset, size, (int)sb.cursize);
		maxLumpEnd = MY_MAX(maxLumpEnd, lumpEnd);
		MSG_InitReadBuffer(&lumpsb[i], sb.data + offset, size);
	}

	bspx_header_t *bspx = BSPX_Setup(sb.data, sb.cursize, maxLumpEnd);


	if (bspx){
		loadmodel.isbspx = true;
		Mod_BSPX_LoadShifts(bspx, sb.data);
		Mod_BSPX_LoadRGBLighting(bspx, sb.data);
	}
		
    
	Mod_Q1BSP_LoadEntities(&lumpsb[LUMP_ENTITIES]);
    Mod_Q1BSP_LoadVertexes(&lumpsb[LUMP_VERTEXES]);
	Mod_Q1BSP_LoadEdges(&lumpsb[LUMP_EDGES]);
	Mod_Q1BSP_LoadSurfedges(&lumpsb[LUMP_SURFEDGES]);
	Mod_Q1BSP_LoadTextures(&lumpsb[LUMP_TEXTURES]);
	Mod_Q1BSP_LoadTexinfo(&lumpsb[LUMP_TEXINFO]);
	Mod_Q1BSP_LoadLighting(&lumpsb[LUMP_LIGHTING]);
	Mod_Q1BSP_LoadPlanes(&lumpsb[LUMP_PLANES]);
    Mod_Q1BSP_LoadFaces(&lumpsb[LUMP_FACES]);
	//Mod_Q1BSP_LoadLeaffaces(&lumpsb[LUMP_MARKSURFACES]);
	//Mod_Q1BSP_LoadVisibility(&lumpsb[LUMP_VISIBILITY]);
	// load submodels before leafs because they contain the number of vis leafs
	Mod_BSP_LoadSubmodels(&lumpsb[LUMP_MODELS], &hullinfo);
	//Mod_Q1BSP_LoadLeafs(&lumpsb[LUMP_LEAFS]);
	//Mod_Q1BSP_LoadNodes(&lumpsb[LUMP_NODES]);

    PRINT("num of edges %i", loadmodel.numedges);
    PRINT("num of surfedges %i", loadmodel.numsurfedges);
	PRINT("num of textures %i", loadmodel.num_textures);
	PRINT("num of vertices %i", loadmodel.vertexCount);
	PRINT("num of leafs %i", loadmodel.num_leafs);
	PRINT("num of leafsurfaces %i", loadmodel.num_leafsurfaces);
	PRINT("num of nodes %i", loadmodel.num_nodes);
	PRINT("num of planes %i", loadmodel.num_planes);
	PRINT("num of num of pvs %i", loadmodel.num_compressedpvs);
	PRINT("num of submodules %i", loadmodel.numsubmodels);
	PRINT("light data %i", loadmodel.num_lightdata);

    //free(loadmodel.edges);
    //free(loadmodel.surfedges);
	//free(loadmodel.texinfo);
	//loadmodel.edges = NULL;
	//loadmodel.surfedges = NULL;
	//loadmodel.texinfo = NULL;
	//free(loadmodel.vertices);
	//free(loadmodel.data_textures);

    memcpy(mod, &loadmodel, sizeof(*mod));


}


//supported lumps (read specs/bspx.txt for more details):
//RGBLIGHTING (.lit)
//LIGHTING_E5BGR9 (hdr lit)
//LIGHTINGDIR (.lux)
//LMSHIFT (lightmap scaling, obsoleted bby DECOUPLED_LM)
//LMOFFSET (lightmap scaling, redundant without LMSHIFT)
//LMSTYLE (for when 4 styles per face are not enough)
//LMSTYLE16 (for when you need more than 256 different lightswitches)
//VERTEXNORMALS (smooth specular)
//BRUSHLIST (no hull size issues)
//ENVMAP (cubemaps)
//SURFENVMAP (cubemaps)
//FACENORMALS (because Quetoo's normals were rejected by ericw for some reason)
//DECOUPLED_LM (upgraded alternative to LM_SHIFT with float scaling and explicit lm sizes)
//LIGHTGRID_OCTREE (lightgrid alternative to floor-based model lighting, but still stuck with 4 8bit ldr styles)

bspx_lump_t* BSPX_FindLump(bspx_header_t *bspxheader, char* lumpname)
{
	int i;
	if (!bspxheader)
		return NULL;

	for (i = 0; i < bspxheader->numlumps; i++)
	{
		if (!strncmp(bspxheader->lumps[i].lumpname, lumpname, 24))
		{
			
			return bspxheader->lumps+i;
		}
	}
	return NULL;
}

bspx_header_t *BSPX_Setup(char *filebase, size_t filelen, size_t lumpsEnd)
{
	size_t i;
	size_t offs = lumpsEnd;
	bspx_header_t *h;


	offs = (offs + 3) & ~3;
	if (offs + sizeof(*h) > filelen)
		h = NULL; /*no space for it*/
	else
	{
		h = (bspx_header_t*)(filebase + offs);

		i = LittleLong(h->numlumps);
		/*verify the header*/
		if (*(int*)h->id != (('B'<<0)|('S'<<8)|('P'<<16)|('X'<<24)) ||
			i < 0 ||
			offs + sizeof(*h) + sizeof(h->lumps[0])*(i-1) > filelen)
			h = NULL;
		else
		{
			h->numlumps = i;
			while(i-->0)
			{
				h->lumps[i].fileofs = LittleLong(h->lumps[i].fileofs);
				h->lumps[i].filelen = LittleLong(h->lumps[i].filelen);
				if (h->lumps[i].fileofs + h->lumps[i].filelen > filelen)
					return NULL;	//some sort of corruption/truncation.

				if (offs < h->lumps[i].fileofs + h->lumps[i].filelen)
					offs = h->lumps[i].fileofs + h->lumps[i].filelen;
			}
		}
	}


	return h;
}


void Mod_BSPX_LoadShifts(bspx_header_t *bspxheader, unsigned char* filedata)
{
	bspx_lump_t* lump = BSPX_FindLump(bspxheader, "LMSHIFT");

	if (lump) {
		loadmodel.lmshifts = filedata + lump->fileofs;
		loadmodel.num_lmshifts = lump->filelen;
		PRINT("loaded BSPX lump: %s", lump->lumpname);
	} else {
		loadmodel.lmshifts = NULL;
		loadmodel.num_lmshifts = 0;
	}

}

void Mod_BSPX_LoadRGBLighting(bspx_header_t *bspxheader, unsigned char* filedata)
{
	bspx_lump_t* lump = BSPX_FindLump(bspxheader, "RGBLIGHTING");

	if (lump) {
		loadmodel.num_lightdata = lump->filelen;
		loadmodel.lightdata = filedata + lump->fileofs;
		PRINT("loaded BSPX lump: %s", lump->lumpname);
	} else {
		loadmodel.lightdata = NULL;
		loadmodel.num_lightdata = 0;
	}
}