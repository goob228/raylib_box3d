#include "model_shared.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#define MULTIPLIER (1.0f / 32.0f)

#define PRINT(val, ...) printf(val "\n", ##__VA_ARGS__)

static model_t loadmodel;



typedef struct {
	float x, y, z;
} vec3;

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




#define MSG_ReadByte(sb) ((sb)->readcount >= (sb)->cursize ? ((sb)->badread = true, -1) : (unsigned char)(sb)->data[(sb)->readcount++])

int BuffLittleLong (const unsigned char *buffer)
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


static void Mod_Q1BSP_LoadVertexes(sizebuf_t *sb)
{
	float* out;
	int			i, count;
	int			structsize = 12;

	if (sb->cursize % structsize)
		PRINT("Mod_Q1BSP_LoadVertexes: funny lump size in %s", loadmodel.name);
	count = sb->cursize / structsize;
	out = (float *)malloc(count * 3 * sizeof(*out));

	loadmodel.vertices = out;
	loadmodel.vertexCount = count;

	for ( i=0 ; i<count ; i++)
	{
		out[0] = MSG_ReadLittleFloat(sb) * MULTIPLIER;
		out[2] = MSG_ReadLittleFloat(sb) * MULTIPLIER; // we should swap Y and Z axis for raylib, cause in quake Z points up
		out[1] = MSG_ReadLittleFloat(sb) * MULTIPLIER;
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
	out = (unsigned int*)malloc(count * 2 * sizeof(*out));

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
	loadmodel.surfedges = (int *)malloc(loadmodel.numsurfedges * structsize);

	for (i = 0;i < loadmodel.numsurfedges;i++)
		loadmodel.surfedges[i] = MSG_ReadLittleLong(sb);
}

static void Mod_Q1BSP_LoadTextures(sizebuf_t *sb)
{
	int i, j, k, num, max, altmax, mtwidth, mtheight, doffset, incomplete, nummiptex = 0, firstskynoshadowtexture = 0;
	unsigned char *data, *mtdata;
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
		loadmodel.data_textures = (texture_t*)calloc(loadmodel.num_textures, sizeof(*loadmodel.data_textures));
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
			loadmodel.data_textures[i] = temptexture;

			PRINT("%s", name);
		}

		// bump it back to where we started parsing
		sb->readcount = (int)watermark;

		
	}
	else
	{
		loadmodel.num_textures = 0;
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
	out = (mtexinfo_t *)malloc( count * sizeof(*out));

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

static void Mod_Q1BSP_LoadFaces(sizebuf_t *sb)
{
    int i, j, count, surfacenum, planenum, smax, tmax, ssize, tsize, firstedge, numedges, totalverts, totaltris, lightmapnumber, lightmapsize, totallightmapsamples, lightmapoffset, texinfoindex, textureindex;
    int structsize = loadmodel.isbsp2 ? 28 : 20;

    if (sb->cursize % structsize)
		PRINT("Mod_Q1BSP_LoadFaces: funny lump size in %s",loadmodel.name);
    count = sb->cursize / structsize;

    totalverts = 0;
	totaltris = 0;
	for (surfacenum = 0;surfacenum < count;surfacenum++)
	{
		if (loadmodel.isbsp2)
			numedges = BuffLittleLong(sb->data + structsize * surfacenum + 12);
		else
			numedges = BuffLittleShort(sb->data + structsize * surfacenum + 8);
		totalverts += numedges;
		totaltris += numedges - 2;
	}

	if (!loadmodel.num_textures) {
		PRINT("Mod_Q1BSP_LoadFaces: no textures, so no meshes in %s", loadmodel.name);
		loadmodel.mesh = NULL;
	} else {
		loadmodel.mesh = (mesh_t*)calloc(loadmodel.num_textures, sizeof(*loadmodel.mesh));
	}
		
	sizebuf_t tempsb = *sb;


    
	loadmodel.indices = (int*)malloc(totaltris*3*4);
	loadmodel.triangleCount = totaltris;

    #define MAX_VERTICES_PER_FACE 64
    unsigned short verticesPerFace[MAX_VERTICES_PER_FACE];  
    

    totalverts = 0;
	totaltris = 0;

	


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
		lightmapoffset = MSG_ReadLittleLong(sb);


		// FIXME: validate edges, texinfo, etc?
		if ((unsigned int) firstedge > (unsigned int) loadmodel.numsurfedges || (unsigned int) numedges > (unsigned int) loadmodel.numsurfedges || (unsigned int) firstedge + (unsigned int) numedges > (unsigned int) loadmodel.numsurfedges)
			PRINT("Mod_Q1BSP_LoadFaces: invalid edge range (firstedge %i, numedges %i, model edges %i)", firstedge, numedges, loadmodel.numsurfedges);
		if (texinfoindex >= loadmodel.numtexinfo)
			PRINT("Mod_Q1BSP_LoadFaces: invalid texinfo range (texinfo index %i, numtexinfo %i)", texinfoindex, loadmodel.numtexinfo);
		textureindex = loadmodel.texinfo[texinfoindex].textureindex;

			

        int num_firstvertex = totalverts;
        int num_vertices = numedges;
        int num_firsttriangle = totaltris;
        int num_triangles = numedges - 2;

		loadmodel.mesh[textureindex].triangleCount += num_triangles;
		

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


		int triangleIndex = 0;

        for (i = 0;i < num_triangles;i++)
		{
			triangleIndex = num_firsttriangle+i;

			loadmodel.indices[triangleIndex*3+0] = verticesPerFace[0];
			loadmodel.indices[triangleIndex*3+1] = verticesPerFace[i+1];
			loadmodel.indices[triangleIndex*3+2] = verticesPerFace[i+2];
		}

    }

	*sb = tempsb; //restore pointers


	for (i = 0; i < loadmodel.num_textures; i++) { 
		if (loadmodel.mesh[i].triangleCount) {
			loadmodel.mesh[i].textureindex = i;
			loadmodel.mesh[i].vertexCount = loadmodel.mesh[i].triangleCount*3;
			loadmodel.mesh[i].vertices = (float*)malloc(loadmodel.mesh[i].vertexCount*3*4);
			loadmodel.mesh[i].normals = (float*)malloc(loadmodel.mesh[i].vertexCount*3*4);
			loadmodel.mesh[i].texcoords = (float*)malloc(loadmodel.mesh[i].vertexCount*2*4);
			
		}
		PRINT("texid: %i tri count: %i", i,  loadmodel.mesh[i].triangleCount );
	}

	PRINT("done mallocing mesh vertices");
	
    totalverts = 0;
	totaltris = 0;

	int texWidth = 0;
	int texHeight = 0;
	float inv_texWidth = 0.0f;
	float inv_texHeight = 0.0f;

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
		lightmapoffset = MSG_ReadLittleLong(sb);



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

		vec3 p1 = (vec3){	
			loadmodel.vertices[verticesPerFace[0]*3],
			loadmodel.vertices[verticesPerFace[0]*3+1],
			loadmodel.vertices[verticesPerFace[0]*3+2]
		};

		vec3 p2 = (vec3){	
			loadmodel.vertices[verticesPerFace[1]*3],
			loadmodel.vertices[verticesPerFace[1]*3+1],
			loadmodel.vertices[verticesPerFace[1]*3+2]
		};

		vec3 p3 = (vec3){	
			loadmodel.vertices[verticesPerFace[2]*3],
			loadmodel.vertices[verticesPerFace[2]*3+1],
			loadmodel.vertices[verticesPerFace[2]*3+2]
		};

		vec3 normal = calc_normal(p1, p2, p3);

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

		float u_coord = 0.0f;
		float v_coord = 0.0f;

		int triangleIndex = 0;

        for (i = 0;i < num_triangles;i++)
		{
			triangleIndex = loadmodel.mesh[textureindex].num_firsttriangle+i;


			p2.x = loadmodel.vertices[verticesPerFace[i+1]*3];
			p2.y = loadmodel.vertices[verticesPerFace[i+1]*3+1];
			p2.z = loadmodel.vertices[verticesPerFace[i+1]*3+2];

			p3.x = loadmodel.vertices[verticesPerFace[i+2]*3];
			p3.y = loadmodel.vertices[verticesPerFace[i+2]*3+1];
			p3.z = loadmodel.vertices[verticesPerFace[i+2]*3+2];

            loadmodel.mesh[textureindex].vertices[(triangleIndex*3+0)*3] = p1.x;
            loadmodel.mesh[textureindex].vertices[(triangleIndex*3+0)*3+1] = p1.y;
            loadmodel.mesh[textureindex].vertices[(triangleIndex*3+0)*3+2] = p1.z;
			
            loadmodel.mesh[textureindex].vertices[(triangleIndex*3+1)*3] = p2.x;
            loadmodel.mesh[textureindex].vertices[(triangleIndex*3+1)*3+1] = p2.y;
            loadmodel.mesh[textureindex].vertices[(triangleIndex*3+1)*3+2] = p2.z;

            loadmodel.mesh[textureindex].vertices[(triangleIndex*3+2)*3] = p3.x;
            loadmodel.mesh[textureindex].vertices[(triangleIndex*3+2)*3+1] = p3.y;
            loadmodel.mesh[textureindex].vertices[(triangleIndex*3+2)*3+2] = p3.z;

			

			loadmodel.mesh[textureindex].normals[(triangleIndex*3+0)*3] = normal.x;
			loadmodel.mesh[textureindex].normals[(triangleIndex*3+0)*3+1] = normal.y;
			loadmodel.mesh[textureindex].normals[(triangleIndex*3+0)*3+2] = normal.z;

			loadmodel.mesh[textureindex].normals[(triangleIndex*3+1)*3] = normal.x;
            loadmodel.mesh[textureindex].normals[(triangleIndex*3+1)*3+1] = normal.y;
            loadmodel.mesh[textureindex].normals[(triangleIndex*3+1)*3+2] = normal.z;

            loadmodel.mesh[textureindex].normals[(triangleIndex*3+2)*3] = normal.x;
            loadmodel.mesh[textureindex].normals[(triangleIndex*3+2)*3+1] = normal.y;
            loadmodel.mesh[textureindex].normals[(triangleIndex*3+2)*3+2] = normal.z;

			loadmodel.mesh[textureindex].texcoords[(triangleIndex*3+0)*2+0] = (dot_product(p1, u_normal) + u_offset)*inv_texWidth;
			loadmodel.mesh[textureindex].texcoords[(triangleIndex*3+0)*2+1] = (dot_product(p1, v_normal) + v_offset)*inv_texHeight;

			loadmodel.mesh[textureindex].texcoords[(triangleIndex*3+1)*2+0] = (dot_product(p2, u_normal) + u_offset)*inv_texWidth;
			loadmodel.mesh[textureindex].texcoords[(triangleIndex*3+1)*2+1] = (dot_product(p2, v_normal) + v_offset)*inv_texHeight;

			loadmodel.mesh[textureindex].texcoords[(triangleIndex*3+2)*2+0] = (dot_product(p3, u_normal) + u_offset)*inv_texWidth;
			loadmodel.mesh[textureindex].texcoords[(triangleIndex*3+2)*2+1] = (dot_product(p3, v_normal) + v_offset)*inv_texHeight;



		}

		loadmodel.mesh[textureindex].num_firsttriangle += num_triangles;

    }

	loadmodel.meshCount = loadmodel.num_textures;

}






void loadBSP(model_t* mod, void* data, void* dataEnd)
{
    int i, j, k;
    sizebuf_t lumpsb[HEADER_LUMPS];
    sizebuf_t sb;

    memcpy(&loadmodel, mod, sizeof(*mod));

	


    MSG_InitReadBuffer(&sb, (unsigned char *)data, (unsigned char *)dataEnd - (unsigned char *)data);

    i = MSG_ReadLittleLong(&sb);

    for (i = 0; i < HEADER_LUMPS; i++) {
        int offset = MSG_ReadLittleLong(&sb);
        int size = MSG_ReadLittleLong(&sb);
        if (offset < 0 || offset + size > sb.cursize)
			PRINT("loadBSP: has invalid lump %i (offset %i, size %i, file size %i)", i, offset, size, (int)sb.cursize);
		MSG_InitReadBuffer(&lumpsb[i], sb.data + offset, size);
	}
    
    Mod_Q1BSP_LoadVertexes(&lumpsb[LUMP_VERTEXES]);
	PRINT("vertices loaded");
	Mod_Q1BSP_LoadEdges(&lumpsb[LUMP_EDGES]);
	PRINT("edges loaded");
	Mod_Q1BSP_LoadSurfedges(&lumpsb[LUMP_SURFEDGES]);
	PRINT("surfedges loaded");
	Mod_Q1BSP_LoadTextures(&lumpsb[LUMP_TEXTURES]);
	PRINT("textures loaded");
	Mod_Q1BSP_LoadTexinfo(&lumpsb[LUMP_TEXINFO]);
	PRINT("texinfo loaded");
    Mod_Q1BSP_LoadFaces(&lumpsb[LUMP_FACES]);
	PRINT("faces loaded");



    PRINT("num of edges %i", loadmodel.numedges);
    PRINT("num of surfedges %i", loadmodel.numsurfedges);
	PRINT("num of textures %i", loadmodel.num_textures);




    loadmodel.numedges = 0;
    loadmodel.numsurfedges = 0;
    free(loadmodel.edges);
    free(loadmodel.surfedges);
	free(loadmodel.texinfo);
	//free(loadmodel.vertices);
	//free(loadmodel.data_textures);

    memcpy(mod, &loadmodel, sizeof(*mod));


}