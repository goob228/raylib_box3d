#ifndef MODEL_SHARED_H
#define MODEL_SHARED_H

#include <stdbool.h>
#include <stdint.h>

#include "bspfile.h"

#define	MAX_QPATH		128

typedef struct {
    int vertexCount;        // Number of vertices stored in arrays
    int triangleCount;      // Number of triangles stored (indexed or not)

    int num_firsttriangle;

    // Vertex attributes data
    float *vertices;        // Vertex position (XYZ - 3 components per vertex) (shader-location = 0)
    float *texcoords;       // Vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
    float *texcoords2;      // Vertex texture second coordinates (UV - 2 components per vertex) (shader-location = 5)
    float *normals;         // Vertex normals (XYZ - 3 components per vertex) (shader-location = 2)
    
    unsigned short *indices; // Vertex indices (in case vertex data comes indexed)

    int textureindex;

} mesh_t;


typedef struct mtexinfo_s
{
	float		vecs[2][4];		// [s/t][xyz offset]
	int			textureindex;
	int			q1flags;
	int			q2flags;			// miptex flags + overrides
	int			q2value;			// light emission, etc
	char		q2texture[32];	// texture name (textures/*.wal)
	int			q2nexttexinfo;	// for animations, -1 = end of chain
}
mtexinfo_t;

typedef struct texture_s
{
    // name
	char name[16];

	// q1bsp
	// size
	unsigned int width, height;

    unsigned int material_id;

} texture_t;

typedef struct {

    char name[128];

    int meshCount;          // Number of meshes
    mesh_t* mesh; 

    float *vertices; 
    int vertexCount;   

	int* indices;
	int triangleCount;

    int numedges;
    unsigned int* edges;

    int numsurfedges;
    int* surfedges;

    int	num_textures;
    texture_t   *data_textures;

    int	numtexinfo;
    mtexinfo_t* texinfo;

    bool isbsp2;
    bool ishlbsp;

} model_t;


typedef struct sizebuf_s
{
	bool	allowoverflow;	///< if false, do a Sys_Error
	bool	overflowed;		///< set to true if the buffer size failed
	unsigned char		*data;
	int			maxsize;
	int			cursize;
	int			readcount;
	bool	badread;		// set if a read goes beyond end of message
} sizebuf_t;



void loadBSP(model_t* mod, void* data, void* dataEnd);


#endif //MODEL_SHARED_H