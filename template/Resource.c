#include "Resource.h"

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#include <box3d/box3d.h>
#include <raylib.h> 
#include <rlgl.h>


#include "Texture.h"
#include "model_shared.h"
#include "MapLoader.h"
#include "Playground.h"
#include "Zone.h"
#include "G_local.h"

#define RESOURCES_SIZE 2048

#define MAP_USAGE 2


model_t g_mapModel = {0};

Shader lightmap_shader = {0};

Shader discard_shader = {0};

Shader skybox_shader = {0};


typedef struct {
    char name[RESOURCE_NAME_SIZE];
    Model model;
    Texture texture;
    b3BodyId bodyid;
    unsigned int usage;
    Res_type type;
    bool occupied;

} Resource;


static Resource resources[RESOURCES_SIZE] = {0};

/*
Model LoadModelFromModel_t(model_t* mt)
{
	int numofmeshes = 0;

	for (int i = 0; i < mt->meshCount; i++) {
        if (!strcmp(mt->data_textures[i].name, "trigger")) {
            continue;
        }
		if (mt->mesh[i].vertices && mt->mesh[i].vertexCount) {
			numofmeshes++;
		}
	}

	Mesh* meshes = (Mesh*)RL_CALLOC(numofmeshes, sizeof(Mesh));


	for (int i = 0, j = 0; i < mt->meshCount; i++) {
		if (mt->mesh[i].vertices && mt->mesh[i].vertexCount) {
            if (!strcmp(mt->data_textures[i].name, "trigger")) {
                continue;
            }
			meshes[j].triangleCount = 		mt->mesh[i].triangleCount;
			meshes[j].vertexCount = 		mt->mesh[i].vertexCount;
			meshes[j].vertices =			mt->mesh[i].vertices;
			meshes[j].normals =			    mt->mesh[i].normals;
			meshes[j].texcoords =			mt->mesh[i].texcoords;
            meshes[j].texcoords2 =			mt->mesh[i].texcoords2;
			UploadMesh(&meshes[j], true);

            meshes[j].vboId[SHADER_LOC_VERTEX_TEXCOORD02] = rlLoadVertexBuffer(meshes[j].texcoords2, meshes[j].vertexCount*2*sizeof(float), false);
            rlEnableVertexArray(meshes[j].vaoId);

            // Index 5 is for texcoords2
            rlSetVertexAttribute(5, 2, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(5);
            rlDisableVertexArray();

			j++;
		}
	}

    Model model = { 0 };

    model.transform = MatrixIdentity();

    model.meshCount = numofmeshes;
    model.meshes = meshes;

    model.materialCount = numofmeshes;
    model.materials = (Material *)RL_CALLOC(model.materialCount, sizeof(Material));
    

    model.meshMaterial = (int *)RL_CALLOC(model.meshCount, sizeof(int));
    

	for (int i = 0; i < model.meshCount; i++) {
		model.meshMaterial[i] = i;
	}
	int texid = 0;
	Resource_key key = (Resource_key){0};

	for (int i = 0, j = 0; i < mt->num_textures, j < model.materialCount; i++) {
        if (!strcmp(mt->data_textures[i].name, "trigger")) {
            continue;
        }
		if (mt->mesh[i].vertices && mt->mesh[i].vertexCount) {
			model.materials[j] = LoadMaterialDefault();
			//int len = strnlen(mt->data_textures[i].name, sizeof(mt->data_textures[i].name));
			//strncpy(&(mt->data_textures[i].name[len]), ".png", 5);
            if (mt->data_textures[i].type == TEXTYPE_SKY) {
                key = loadTextureResource("\\sky");
                model.materials[j].shader = skybox_shader;
                model.materials[j].maps[MATERIAL_MAP_CUBEMAP].texture = getTextureResource(&key);
            } else {
                key = loadTextureResource(mt->data_textures[i].name);
                
                if (mt->data_textures[i].name[0] == '{') {
                    model.materials[j].shader = discard_shader;
                } else {
                    model.materials[j].shader = lightmap_shader;
                }
                   

                model.materials[j].maps[MATERIAL_MAP_ALBEDO].texture = getTextureResource(&key);
                key = loadTextureResource("\\light");
                model.materials[j].maps[MATERIAL_MAP_METALNESS].texture = getTextureResource(&key);
                SetTextureWrap(model.materials[j].maps[MATERIAL_MAP_ALBEDO].texture, TEXTURE_WRAP_REPEAT);
            }
			    
			
			
			j++;
		}
	}


    return model;
}*/

void loadSubModelsToResource(model_t* mt)
{
    int beforemark = Hunk_LowMark();

    typedef struct {
        int mesh_idx;
        int num_vertices;
    } VertPerTexture;

    mesh_t* basemesh = mt->mesh;
    Resource_key texkey = {0};


    for (int smid = 0; smid < mt->numsubmodels; smid++) {
        mmodel_t* sm = mt->submodels + smid;

        Model md = {0};

        

        md.transform = MatrixIdentity();
        

        msurface_t* surface;
        int i;

        int numvertices = 0;

        VertPerTexture* vpt = (VertPerTexture*)Hunk_Alloc(mt->num_textures*sizeof(VertPerTexture));

        int numusedtextures = 0;

        for (i = 0, surface = mt->data_surfaces+sm->firstface; i < sm->numfaces; i++, surface++) {
            if (vpt[surface->tex_idx].num_vertices == 0 && surface->num_vertices){
                numusedtextures++;
            }
            vpt[surface->tex_idx].num_vertices += surface->num_vertices;
        }

        Mesh* meshes = (Mesh*)RL_CALLOC(numusedtextures, sizeof(Mesh));   

        md.meshCount = numusedtextures;
        md.meshes = meshes;
        md.materialCount = numusedtextures;
        md.materials = (Material*)RL_CALLOC(numusedtextures, sizeof(Material));
        md.meshMaterial = (int*)RL_CALLOC(numusedtextures, sizeof(int));

        int j = 0;
        for (i = 0; i < mt->num_textures; i++) {
            if (vpt[i].num_vertices)  {
                vpt[i].mesh_idx = j;
                meshes[j].vertices = (float*)Hunk_AllocNoFill(vpt[i].num_vertices*3*sizeof(float));
                meshes[j].texcoords = (float*)Hunk_AllocNoFill(vpt[i].num_vertices*2*sizeof(float));
                meshes[j].texcoords2 = (float*)Hunk_AllocNoFill(vpt[i].num_vertices*2*sizeof(float));
                //meshes[0].vertices = (float*)Hunk_AllocNoFill(numvertices*3*sizeof(float));
                meshes[j].vertexCount = 0;
                meshes[j].triangleCount = vpt[i].num_vertices/3;

                md.materials[j] = LoadMaterialDefault();
        
                md.meshMaterial[j] = j;

                if (mt->data_textures[i].type == TEXTYPE_SKY) {
                    md.materials[j].shader = skybox_shader;
                    texkey = loadTextureResource("\\sky");
                    md.materials[j].maps[MATERIAL_MAP_CUBEMAP].texture = getTextureResource(&texkey);
                } else {
                    md.materials[j].shader = lightmap_shader; 
                    texkey = loadTextureResource(mt->data_textures[i].name);
                    md.materials[j].maps[MATERIAL_MAP_ALBEDO].texture = getTextureResource(&texkey);
                    texkey = loadTextureResource("\\light");
                    md.materials[j].maps[MATERIAL_MAP_METALNESS].texture = getTextureResource(&texkey);
                }
                j++;
            }
        }

        int firstvert = 0;
        int mesh_idx = 0;
        
        for (i = 0, surface = mt->data_surfaces+sm->firstface; i < sm->numfaces; i++, surface++) {
            mesh_idx = vpt[surface->tex_idx].mesh_idx;
            firstvert = meshes[mesh_idx].vertexCount;
            memcpy(meshes[mesh_idx].vertices+firstvert*3, basemesh->vertices + surface->num_firstvertex*3, surface->num_vertices*3*sizeof(float) );
            memcpy(meshes[mesh_idx].texcoords+firstvert*2, basemesh->texcoords + surface->num_firstvertex*2, surface->num_vertices*2*sizeof(float) );
            memcpy(meshes[mesh_idx].texcoords2+firstvert*2, basemesh->texcoords2 + surface->num_firstvertex*2, surface->num_vertices*2*sizeof(float) );
            meshes[mesh_idx].vertexCount += surface->num_vertices;
        }

        for (i = 0;i < numusedtextures; i++) {
            UploadMesh(meshes+i, false);
            meshes[i].vertices = NULL;
            meshes[i].texcoords = NULL;
            meshes[i].texcoords2 = NULL;
        }

        Hunk_FreeToLowMark(beforemark);

        Resource_key mapkey;

        if (smid == 0) {
            mapkey = setModelResource(md,"\\map");
            
        } else {
            mapkey = setModelResource(md,TextFormat("*%i", smid));
        }
        
        setUsageResource(&mapkey, MAP_USAGE);

        



    }

}

void loadMapResource(const char* mapname)
{
    model_t mapMod = loadMyMap(mapname);
    g_mapModel = mapMod;
	
	//Model mapModel = LoadModelFromModel_t(&mapMod);
    //Model mapModel = {0};
    //Resource_key mapkey = setModelResource(mapModel,"\\map");

    loadSubModelsToResource(&mapMod);

    //setUsageResource(&mapkey, MAP_USAGE);

    b3MeshDef def = {0};
	def.vertices      = (b3Vec3*)mapMod.vertices;
	def.vertexCount   = mapMod.vertexCount;
	def.indices       = mapMod.indices;
	def.triangleCount = mapMod.triangleCount;
	def.weldVertices  = true;
	def.identifyEdges = true;            // adjacency info for smooth inter-triangle normals
	def.weldTolerance = 0.01f;
	
	
	
	b3MeshData* mesh = b3CreateMesh(&def, NULL, 0);

	b3BodyDef bodyDef = b3DefaultBodyDef();
	b3BodyId body = b3CreateBody( g_worldid, &bodyDef );

	b3ShapeDef shapeDef = b3DefaultShapeDef();
	b3SurfaceMaterial materials[3];
	materials[0] = (b3SurfaceMaterial){ 0.6f, 0.0f, 0 };
	materials[1] = (b3SurfaceMaterial){ 0.6f, 1.0f, 1 };
	materials[2] = (b3SurfaceMaterial){ 0.1f, 0.0f, 2 };
	shapeDef.materials = materials;
	shapeDef.materialCount = 3;

	b3CreateMeshShape( body, &shapeDef, mesh, b3Vec3_one );

    parseEntities(mapMod.entities);
}

void initResources()
{
    
    resources[0].texture = LoadTexture("");
    resources[0].model = LoadModelFromMesh(GenMeshCube(5.0f, 5.0f, 5.0f));
    resources[0].occupied = true;

    //loadPalette("res/palette.lmp");

    FilePathList files = LoadDirectoryFiles("res");

    for (int i = 0; i < files.count; i++) {
        if (IsFileExtension(files.paths[i], ".wad")) {
            TraceLog(LOG_INFO, "file extensions %s", files.paths[i]);
            W_LoadTextureWadFile(files.paths[i], 0);
        }
    }

    UnloadDirectoryFiles(files);

    
	
    if (!IsShaderValid(lightmap_shader))
        lightmap_shader = LoadShader(TextFormat("res/shaders/shader.vs"), TextFormat("res/shaders/shader.fs"));

    if (!IsShaderValid(discard_shader))
        discard_shader = LoadShader(TextFormat("res/shaders/shader_discard.vs"), TextFormat("res/shaders/shader_discard.fs"));

    if (!IsShaderValid(skybox_shader))
        skybox_shader = LoadShader(TextFormat("res/shaders/skybox.vs"),TextFormat("res/shaders/skybox.fs"));
        SetShaderValue(skybox_shader, GetShaderLocation(skybox_shader, "environmentMap"), (int[1]){ MATERIAL_MAP_CUBEMAP }, SHADER_UNIFORM_INT);
        SetShaderValue(skybox_shader, GetShaderLocation(skybox_shader, "doGamma"), (int[1]){ 0 }, SHADER_UNIFORM_INT);
        SetShaderValue(skybox_shader, GetShaderLocation(skybox_shader, "vflipped"), (int[1]){ 0 }, SHADER_UNIFORM_INT);

    Image image = LoadImage("res/skybox.png");
    setTextureResource(LoadTextureCubemap(image, CUBEMAP_LAYOUT_AUTO_DETECT), "\\sky");
    UnloadImage(image);

    
    loadMapResource("fall.bsp");
    

}



Resource_key loadTextureResource(const char* name)
{
    Resource_key key = (Resource_key){0};
    int tempid = 0;
    for (int i = 1; i < RESOURCES_SIZE; i++) {
        if (tempid == 0 && !resources[i].occupied) {
            tempid = i;
        }
        if (resources[i].occupied && resources[i].type == RES_TEXTURE && strncmp(name, resources[i].name, RESOURCE_NAME_SIZE) == 0) {
            strncpy(key.name, name, RESOURCE_NAME_SIZE );
            key.name[RESOURCE_NAME_SIZE-1] = 0;
            key.id = i;
            return key;
        }
    }

    if (tempid == 0) {
        return (Resource_key){0};
    }
    Texture tex;
    if (!FileExists(name)) {
        int success = 0;
        tex = loadLmpTexture(name, &success);
        if (!success) {
            TraceLog(LOG_WARNING, "Resource.c: didnt find texture [%s]", name);
            return (Resource_key){0};
        }
        
    } else {
        tex = LoadTexture(name);
    }


    resources[tempid].type = RES_TEXTURE;
    resources[tempid].texture =  tex;
    resources[tempid].occupied = true;
    strncpy(resources[tempid].name, name, RESOURCE_NAME_SIZE);
    key.id = tempid;
    strncpy(key.name, name, RESOURCE_NAME_SIZE);

    return key;  

}

Resource_key setTextureResource(Texture texture, const char* name)
{
    Resource_key key = (Resource_key){0};
    int tempid = 0;
    for (int i = 1; i < RESOURCES_SIZE; i++) {
        if (tempid == 0 && !resources[i].occupied) {
            tempid = i;
            break;
        }
    }

    if (tempid == 0) {
        return (Resource_key){0};
    }


    resources[tempid].type = RES_TEXTURE;
    resources[tempid].texture =  texture;
    resources[tempid].occupied = true;
    strncpy(resources[tempid].name, name, RESOURCE_NAME_SIZE);
    key.id = tempid;
    strncpy(key.name, name, RESOURCE_NAME_SIZE);

    return key;  

}

Texture getTextureResource(Resource_key* key)
{

    if (resources[key->id].occupied && strncmp(resources[key->id].name, key->name, RESOURCE_NAME_SIZE) == 0) {
        return resources[key->id].texture;
    }


    return resources[0].texture;
}

Resource_key loadModelResource(const char* name)
{
    Resource_key key = (Resource_key){0};
    int tempid = 0;
    for (int i = 1; i < RESOURCES_SIZE; i++) {
        if (tempid == 0 && !resources[i].occupied) {
            tempid = i;
        }
        if (resources[i].occupied && resources[i].type == RES_MODEL && strncmp(name, resources[i].name, RESOURCE_NAME_SIZE) == 0 ) {
            strncpy(key.name, name, RESOURCE_NAME_SIZE );
            key.name[RESOURCE_NAME_SIZE-1] = 0;
            key.id = i;
            return key;
        }
    }

    if (tempid == 0) {
        return (Resource_key){0};
    }


    resources[tempid].type = RES_MODEL;
    resources[tempid].model =  LoadModel(name);
    resources[tempid].occupied = true;
    strncpy(resources[tempid].name, name, RESOURCE_NAME_SIZE);
    key.id = tempid;
    strncpy(key.name, name, RESOURCE_NAME_SIZE);

    

    return key;  

}

Resource_key setModelResource(Model model, const char* name)
{
    Resource_key key = (Resource_key){0};
    int tempid = 0;
    for (int i = 1; i < RESOURCES_SIZE; i++) {
        if (tempid == 0 && !resources[i].occupied) {
            tempid = i;
            break;
        }
    }

    if (tempid == 0) {
        return (Resource_key){0};
    }


    resources[tempid].type = RES_MODEL;
    resources[tempid].model =  model;
    resources[tempid].occupied = true;
    strncpy(resources[tempid].name, name, RESOURCE_NAME_SIZE);
    key.id = tempid;
    strncpy(key.name, name, RESOURCE_NAME_SIZE);

    

    return key;  

}


Model getModelResource(Resource_key* key)
{

    if (resources[key->id].occupied && strncmp(resources[key->id].name, key->name, RESOURCE_NAME_SIZE) == 0) {
        return resources[key->id].model;
    }


    return resources[0].model;
}


void setUsageResource(Resource_key* key, int usage)
{
    if (resources[key->id].occupied && strncmp(resources[key->id].name, key->name, RESOURCE_NAME_SIZE) == 0) {
        resources[key->id].usage = usage;
    }
}


void clearResources()
{
    Model model = (Model){0};
    Texture texture = (Texture){0};
    for (int i = 0; i < RESOURCES_SIZE; i++) {
        if (resources[i].occupied) {
            switch (resources[i].type)
            {
            case RES_MODEL:
                model = resources[i].model;
                if (resources[i].usage != MAP_USAGE) {
                    
                    UnloadModel(model);
                } else {
                    for (int i = 0; i < model.meshCount; i++) {
                        
                        model.meshes[i].triangleCount = 	0;
                        model.meshes[i].vertexCount = 		0;
                        model.meshes[i].vertices =			NULL;
                        model.meshes[i].normals =			NULL;
                        model.meshes[i].texcoords =			NULL;
                        model.meshes[i].texcoords2 =		NULL;
                    }
                    UnloadModel(model);
                }
                break;

            case RES_TEXTURE:
                texture = resources[i].texture;
                UnloadTexture(texture);
                break;
            
            default:
                break;
            }
        }
    }

    Hunk_FreeToLowMark(0);

    if (b3World_IsValid(g_worldid)) {
        b3DestroyWorld(g_worldid);
    }

    memset(resources, 0, sizeof(resources));
    

}