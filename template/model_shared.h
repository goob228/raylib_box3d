#ifndef MODEL_SHARED_H
#define MODEL_SHARED_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>



#include "bspfile.h"

#define	MAX_QPATH		128

typedef struct atlase_s {
	int ax;
	int ay;
	int wx;
	int wy;

	int offset;

} atlase_t;

typedef struct {
    int vertexCount;        // Number of vertices stored in arrays
    int triangleCount;      // Number of triangles stored (indexed or not)
	int faceCount;

    int num_firsttriangle;
    int num_firstvertex;
	int num_firstface;

    // Vertex attributes data
    float *vertices;        // Vertex position (XYZ - 3 components per vertex) (shader-location = 0)
    float *texcoords;       // Vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
    float *texcoords2;      // Vertex texture second coordinates (UV - 2 components per vertex) (shader-location = 5)
    float *normals;         // Vertex normals (XYZ - 3 components per vertex) (shader-location = 2)
	atlase_t* atlases;             // Face atlas (1 component per face)
    
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

typedef enum {
	TEXTYPE_DEFAULT,
	TEXTYPE_CUTOUT,
	TEXTYPE_SKY,
	TEXTYPE_LAVA,
	TEXTYPE_SLIME,
	TEXTYPE_TELE,
	TEXTYPE_WATER,

	TEXTYPE_COUNT,

	TEXTYPE_FIRSTLIQUID = TEXTYPE_LAVA,
	TEXTYPE_LASTLIQUID = TEXTYPE_WATER,
	TEXTYPE_NUMLIQUIDS = TEXTYPE_LASTLIQUID + 1 - TEXTYPE_FIRSTLIQUID,
} textype_t;

typedef struct texture_s
{
    // name
	char name[32];

	// q1bsp
	// size
	unsigned int width, height;

    unsigned int material_id;

	textype_t type;

} texture_t;

// plane_t structure
typedef struct mplane_s
{
	union
	{
		struct
		{
			float normal[3];
			float dist;
		};
		float normal_and_dist[4];
	};
	// for texture axis selection and fast side tests
	int type; // set by PlaneClassify()
	int signbits; // set by PlaneClassify()
}
mplane_t;

/// <summary>
///  describes the textures to use on a range of triangles in the model, and mins/maxs (AABB) for culling.
/// </summary>
typedef struct msurface_s
{
	/// range of triangles and vertices in model->surfmesh
	int num_triangles; // triangles
	int num_firsttriangle; // first element is this *3
	int num_vertices; // length of the range referenced by elements
	int num_firstvertex; // min vertex referenced by elements

	int tex_idx;

	// the following fields are used situationally and are not part of rendering in typical usage

	/// bounding box for onscreen checks
	float mins[3];
	float maxs[3];

	/// lightmaptexture rebuild information not used in q3bsp
	//msurface_lightmapinfo_t* lightmapinfo; // q1bsp
	/// fog volume info in q3bsp
	//struct q3deffect_s* effect; // q3bsp

	// used by Mod_Mesh_Finalize when building sortedmodelsurfaces
	bool included;
} msurface_t;

typedef struct mleaf_s
{
	//this part shared between node and leaf
	mplane_t *plane; // == NULL
	struct mnode_s *parent;
	struct mportal_s *portals;
	// for bounding box culling
	float mins[3];
	float maxs[3];
	// supercontents from all brushes inside this node or leaf
	int combinedsupercontents;

	// this part unique to leaf
	// common
	int clusterindex; // -1 is not in pvs, >= 0 is pvs bit number
	int areaindex; // q3bsp
	int containscollisionsurfaces; // indicates whether the leafsurfaces contains q3 patches
	int numleafsurfaces;
	int *firstleafsurface;
	int numleafbrushes; // q3bsp
	int *firstleafbrush; // q3bsp
	unsigned char ambient_sound_level[NUM_AMBIENTS]; // q1bsp
	int contents; // q1bsp: // TODO: remove (only used temporarily during loading when making collision hull 0)
	int portalmarkid; // q1bsp // used by see-polygon-through-portals visibility checker
}
mleaf_t;

typedef struct mnode_s
{
	//this part shared between node and leaf
	mplane_t *plane; // != NULL
	struct mnode_s *parent;
	struct mportal_s *portals;
	// for bounding box culling
	float mins[3];
	float maxs[3];
	// supercontents from all brushes inside this node or leaf
	int combinedsupercontents;

	// this part unique to node
	struct mnode_s *children[2];

	// q1bsp specific
	unsigned int firstsurface;
	unsigned int numsurfaces;
}
mnode_t;

typedef struct {

    char name[MAX_QPATH];

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

	int numsubmodels;
	mmodel_t		*submodels;

    int	num_textures;
    texture_t   *data_textures;

    int	numtexinfo;
    mtexinfo_t* texinfo;

	int				num_lightdata;
	unsigned char			*lightdata;

	int				num_surfaces;
	msurface_t		*data_surfaces;

	int num_planes;
	mplane_t *data_planes;

		int num_nodes;
	mnode_t *data_nodes;

	int num_leafsurfaces;
	int *data_leafsurfaces;

	int				num_compressedpvs;
	unsigned char			*data_compressedpvs;

		// number of actual leafs (including 0 which is solid)
	int num_leafs;
	mleaf_t *data_leafs;

	int light_width;
	int light_height;
	unsigned char* lightTexture;

	bool lightOverflow;

	char *entities;

    bool isbsp2;
    bool ishlbsp;
	bool isbsp2rmqe;


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

typedef int64_t fs_offset_t;

typedef struct qpic_s
{
	int			width, height;
	unsigned char		data[4];			// variably sized
} qpic_t;



typedef struct wadinfo_s
{
	char		identification[4];		// should be WAD2 or 2DAW
	int			numlumps;
	int			infotableofs;
} wadinfo_t;

typedef struct lumpinfo_s
{
	int			filepos;
	int			disksize;
	int			size;					// uncompressed
	char		type;
	char		compression;
	char		pad1, pad2;
	char		name[16];				// must be null terminated
} lumpinfo_t;


typedef struct mwad_s
{
	FILE *file;
	int numlumps;
	lumpinfo_t *lumps;
	int version;
}
mwad_t;

typedef struct wadstate_s
{
	unsigned char *gfx_base;
	mwad_t gfx;
}
wadstate_t;

extern int model_shared_image_width, model_shared_image_height;

void loadBSP(model_t* mod, void* data, void* dataEnd);

void W_LoadTextureWadFile (char *filename, int complain);

unsigned char *W_GetTextureBGRA(char *name);

void loadPalette(const char* filename);

void parseEntities(const char* str);


#endif //MODEL_SHARED_H