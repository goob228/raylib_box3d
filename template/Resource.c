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

#define RESOURCES_SIZE 1024


Tree mapTree = {0};

model_t g_mapModel = {0};

Shader lightmap_shader = {0};

Shader discard_shader = {0};

Shader skybox_shader = {0};

int beforeMapMark = 0;

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

#define MAX_UNSIGNED_SHORT 65535

#define OCTREE_DEEP 4

#define MY_MAX(a, b) (a > b ? a : b)
#define MY_MIN(a, b) (a < b ? a : b)




bool isTriangleInsideBox(float p1[3], float p2[3], float p3[3], BoundingBox* box) 
{
    float min[3] = {MY_MIN(MY_MIN(p1[0], p2[0]), p3[0]), MY_MIN(MY_MIN(p1[1], p2[1]), p3[1]), MY_MIN(MY_MIN(p1[2], p2[2]), p3[2])};
    float max[3] = {MY_MAX(MY_MAX(p1[0], p2[0]), p3[0]), MY_MAX(MY_MAX(p1[1], p2[1]), p3[1]), MY_MAX(MY_MAX(p1[2], p2[2]), p3[2])};

    return      (min[0] <= box->max.x && max[0] >= box->min.x) &&
                (min[1] <= box->max.y && max[1] >= box->min.y) &&
                (min[2] <= box->max.z && max[2] >= box->min.z);
}

OctreeNode* newOctreeNode(int meshCount)
{
    OctreeNode* node = (OctreeNode*)Hunk_Alloc(sizeof(OctreeNode));
    node->meshCount = meshCount;
    node->triangleIndexPerMesh = (int*)Hunk_Alloc(sizeof(int) * meshCount);
    node->triangleCountPerMesh = (int*)Hunk_Alloc(sizeof(int) * meshCount);
    return node;
}

void swapTriangles(unsigned short tri1[3], unsigned short tri2[3])
{
    unsigned short temp[3];

    memmove(temp, tri1, sizeof(unsigned short)*3);
    memmove(tri1, tri2, sizeof(unsigned short)*3);
    memmove(tri2, temp, sizeof(unsigned short)*3);

    /*
    unsigned short temp;

    temp = tri1[0];
    tri1[0] = tri2[0];
    tri2[0] = temp;

    temp = tri1[1];
    tri1[1] = tri2[1];
    tri2[1] = temp;

    temp = tri1[2];
    tri1[2] = tri2[2];
    tri2[2] = temp;
    */

}

Material drawingMaterial;
Mesh* drawingMesh;
Matrix drawingTransform;
int meshId;

void RecurseBuildOctree(OctreeNode* node);

void RecurseBuildOctree(OctreeNode* node, int trifirst, int tricount)
{
    if (!node) return;


    float* verts = drawingMesh->vertices;
    int triangleCount = 0;
    int indIndex;
    for (int triangleIndex = trifirst; triangleIndex < trifirst+tricount; triangleIndex++) {
        indIndex = triangleIndex*3;
        int i1 = 3 * drawingMesh->indices[indIndex] ;
        int i2 = 3 * drawingMesh->indices[indIndex+1];
        int i3 = 3 * drawingMesh->indices[indIndex+2];
        if (isTriangleInsideBox(verts+i1, verts+i2, verts+i3, &(node->box))) {
            if (trifirst + triangleCount != triangleIndex) {
                swapTriangles(&(drawingMesh->indices[indIndex]), &(drawingMesh->indices[(trifirst + triangleCount)*3]));
            }
            triangleCount++;
        }
    }


    node->triangleIndexPerMesh[meshId] = trifirst;
    node->triangleCountPerMesh[meshId] = triangleCount;


    if (node->deepNess >= 1) {
        node->isLeaf = 1;
        return;
    }

    node->isLeaf = 0;

    if (node->children[0] == NULL){

        OctreeNode* child;

        Vector3 center = (Vector3){ (node->box.min.x + node->box.max.x) * 0.5f, (node->box.min.y + node->box.max.y) * 0.5f, (node->box.min.z + node->box.max.z) * 0.5f };


        for (int i = 0; i < 8; i++) {
            node->children[i] = newOctreeNode(node->meshCount);
            node->children[i]->deepNess = node->deepNess + 1;

            child = node->children[i];

            child->box.min.x = (i & 1) ? center.x : node->box.min.x;
            child->box.max.x = (i & 1) ? node->box.max.x : center.x;

            child->box.min.y = (i & 2) ? center.y : node->box.min.y;
            child->box.max.y = (i & 2) ? node->box.max.y : center.y;

            child->box.min.z = (i & 4) ? center.z : node->box.min.z;
            child->box.max.z = (i & 4) ? node->box.max.z : center.z;
        }
    }



    int trioffset = trifirst;

    int triCounter = tricount;

    for (int i = 0; i < 8; i++) {
        if (node->children[i]){
            RecurseBuildOctree(node->children[i], trioffset, triCounter);
            trioffset += node->children[i]->triangleCountPerMesh[meshId];
            triCounter -= node->children[i]->triangleCountPerMesh[meshId];
        }
    }

}


Tree buildOctreeFromModel(Model model, BoundingBox box)
{
    Tree tree = {0};

    
    tree.meshCount = model.meshCount;
    tree.meshes = model.meshes;
    tree.root = newOctreeNode(tree.meshCount);
    tree.root->box = box;

    OctreeNode* node = tree.root;
    node->deepNess = 1;

    int indIndex = 0;
    int triangleCount = 0;
    int triangleIndex = 0;

    Mesh* mesh;
    int meshIdx;

    for (meshIdx = 0, mesh = tree.meshes; meshIdx < tree.meshCount; meshIdx++, mesh++) {
        
        drawingMesh = mesh;
        meshId = meshIdx;

        RecurseBuildOctree(node, 0, mesh->triangleCount);

    }


    return tree;
}

static void drawMyMesh(Mesh mesh, Material mat, Matrix transform, int firstTri, int triCount)
{

    if (triCount == 0) return;

    rlEnableShader(mat.shader.id);

    if (mat.shader.locs[SHADER_LOC_COLOR_DIFFUSE] != -1)
    {
        float values[4] = {
            (float)mat.maps[MATERIAL_MAP_DIFFUSE].color.r/255.0f,
            (float)mat.maps[MATERIAL_MAP_DIFFUSE].color.g/255.0f,
            (float)mat.maps[MATERIAL_MAP_DIFFUSE].color.b/255.0f,
            (float)mat.maps[MATERIAL_MAP_DIFFUSE].color.a/255.0f
        };

        rlSetUniform(mat.shader.locs[SHADER_LOC_COLOR_DIFFUSE], values, SHADER_UNIFORM_VEC4, 1);
    }


    // Get a copy of current matrices to work with,
    // in case stereo render is required, and they need to be modified
    // NOTE: At this point the modelview matrix contains the view matrix (camera)
    // That's because BeginMode3D() sets it and there is no model-drawing function
    // that modifies it, all use rlPushMatrix() and rlPopMatrix()
    Matrix matModel = MatrixIdentity();
    Matrix matView = rlGetMatrixModelview();
    Matrix matModelView = MatrixIdentity();
    Matrix matProjection = rlGetMatrixProjection();

    // Upload view and projection matrices (if locations available)
    if (mat.shader.locs[SHADER_LOC_MATRIX_VIEW] != -1) rlSetUniformMatrix(mat.shader.locs[SHADER_LOC_MATRIX_VIEW], matView);
    if (mat.shader.locs[SHADER_LOC_MATRIX_PROJECTION] != -1) rlSetUniformMatrix(mat.shader.locs[SHADER_LOC_MATRIX_PROJECTION], matProjection);

    // Accumulate several model transformations:
    //    transform: model transformation provided (includes DrawModel() params combined with model.transform)
    //    rlGetMatrixTransform(): rlgl internal transform matrix due to push/pop matrix stack
    matModel = MatrixMultiply(transform, rlGetMatrixTransform());

    // Model transformation matrix is sent to shader uniform location: SHADER_LOC_MATRIX_MODEL
    if (mat.shader.locs[SHADER_LOC_MATRIX_MODEL] != -1) rlSetUniformMatrix(mat.shader.locs[SHADER_LOC_MATRIX_MODEL], matModel);

    // Get model-view matrix
    matModelView = MatrixMultiply(matModel, matView);

    // Upload model normal matrix (if locations available)
    if (mat.shader.locs[SHADER_LOC_MATRIX_NORMAL] != -1) rlSetUniformMatrix(mat.shader.locs[SHADER_LOC_MATRIX_NORMAL], MatrixTranspose(MatrixInvert(matModel)));
    //-----------------------------------------------------

    // Bind active texture maps (if available)
    for (int i = 0; i < 12; i++)
    {
        if (mat.maps[i].texture.id > 0)
        {
            // Select current shader texture slot
            rlActiveTextureSlot(i);

            // Enable texture for active slot
            if ((i == MATERIAL_MAP_IRRADIANCE) ||
                (i == MATERIAL_MAP_PREFILTER) ||
                (i == MATERIAL_MAP_CUBEMAP)) rlEnableTextureCubemap(mat.maps[i].texture.id);
            else rlEnableTexture(mat.maps[i].texture.id);

            rlSetUniform(mat.shader.locs[SHADER_LOC_MAP_DIFFUSE + i], &i, SHADER_UNIFORM_INT, 1);
        }
    }

    // Try binding vertex array objects (VAO) or use VBOs if not possible
    // WARNING: UploadMesh() enables all vertex attributes available in mesh and sets default attribute values
    // for shader expected vertex attributes that are not provided by the mesh (i.e. colors)
    // This could be a dangerous approach because different meshes with different shaders can enable/disable some attributes
    if (!rlEnableVertexArray(mesh.vaoId))
    {
        // Bind mesh VBO data: vertex position (shader-location = 0)
        rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION]);
        rlSetVertexAttribute(mat.shader.locs[SHADER_LOC_VERTEX_POSITION], 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(mat.shader.locs[SHADER_LOC_VERTEX_POSITION]);

        // Bind mesh VBO data: vertex texcoords (shader-location = 1)
        rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD]);
        rlSetVertexAttribute(mat.shader.locs[SHADER_LOC_VERTEX_TEXCOORD01], 2, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(mat.shader.locs[SHADER_LOC_VERTEX_TEXCOORD01]);

        if (mat.shader.locs[SHADER_LOC_VERTEX_NORMAL] != -1)
        {
            // Bind mesh VBO data: vertex normals (shader-location = 2)
            rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL]);
            rlSetVertexAttribute(mat.shader.locs[SHADER_LOC_VERTEX_NORMAL], 3, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(mat.shader.locs[SHADER_LOC_VERTEX_NORMAL]);
        }

        // Bind mesh VBO data: vertex colors (shader-location = 3, if available)
        if (mat.shader.locs[SHADER_LOC_VERTEX_COLOR] != -1)
        {
            if (mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR] != 0)
            {
                rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR]);
                rlSetVertexAttribute(mat.shader.locs[SHADER_LOC_VERTEX_COLOR], 4, RL_UNSIGNED_BYTE, 1, 0, 0);
                rlEnableVertexAttribute(mat.shader.locs[SHADER_LOC_VERTEX_COLOR]);
            }
            else
            {
                // Set default value for defined vertex attribute in shader but not provided by mesh
                // WARNING: It could result in GPU undefined behaviour
                float value[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
                rlSetVertexAttributeDefault(mat.shader.locs[SHADER_LOC_VERTEX_COLOR], value, SHADER_ATTRIB_VEC4, 4);
                rlDisableVertexAttribute(mat.shader.locs[SHADER_LOC_VERTEX_COLOR]);
            }
        }

        // Bind mesh VBO data: vertex tangents (shader-location = 4, if available)
        if (mat.shader.locs[SHADER_LOC_VERTEX_TANGENT] != -1)
        {
            rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT]);
            rlSetVertexAttribute(mat.shader.locs[SHADER_LOC_VERTEX_TANGENT], 4, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(mat.shader.locs[SHADER_LOC_VERTEX_TANGENT]);
        }

        // Bind mesh VBO data: vertex texcoords2 (shader-location = 5, if available)
        if (mat.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02] != -1)
        {
            rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2]);
            rlSetVertexAttribute(mat.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02], 2, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(mat.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02]);
        }

    
        if (mesh.indices != NULL) rlEnableVertexBufferElement(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES]);
    }

    int eyeCount = 1;
    if (rlIsStereoRenderEnabled()) eyeCount = 2;

    for (int eye = 0; eye < eyeCount; eye++)
    {
        // Calculate model-view-projection matrix (MVP)
        Matrix matModelViewProjection = MatrixIdentity();
        if (eyeCount == 1) matModelViewProjection = MatrixMultiply(matModelView, matProjection);
        else
        {
            // Setup current eye viewport (half screen width)
            rlViewport(eye*rlGetFramebufferWidth()/2, 0, rlGetFramebufferWidth()/2, rlGetFramebufferHeight());
            matModelViewProjection = MatrixMultiply(MatrixMultiply(matModelView, rlGetMatrixViewOffsetStereo(eye)), rlGetMatrixProjectionStereo(eye));
        }

        // Send combined model-view-projection matrix to shader
        rlSetUniformMatrix(mat.shader.locs[SHADER_LOC_MATRIX_MVP], matModelViewProjection);

        // Draw mesh
        if (mesh.indices != NULL) rlDrawVertexArrayElements(firstTri*3, triCount*3, 0);
        else rlDrawVertexArray(0, mesh.vertexCount);
    }

    // Unbind all bound texture maps
    for (int i = 0; i < 12; i++)
    {
        if (mat.maps[i].texture.id > 0)
        {
            // Select current shader texture slot
            rlActiveTextureSlot(i);

            // Disable texture for active slot
            if ((i == MATERIAL_MAP_IRRADIANCE) ||
                (i == MATERIAL_MAP_PREFILTER) ||
                (i == MATERIAL_MAP_CUBEMAP)) rlDisableTextureCubemap();
            else rlDisableTexture();
        }
    }

    // Disable all possible vertex array objects (or VBOs)
    rlDisableVertexArray();
    rlDisableVertexBuffer();
    rlDisableVertexBufferElement();

    // Disable shader program
    rlDisableShader();

    // Restore rlgl internal modelview and projection matrices
    rlSetMatrixModelview(matView);
    rlSetMatrixProjection(matProjection);
}



void RecurseDrawOctree(OctreeNode* node);

void RecurseDrawOctree(OctreeNode* node)
{
    if (!node) return;

    if (node->visible == false) return;

    if (node->isLeaf) {
        drawMyMesh(*drawingMesh, drawingMaterial, drawingTransform, node->triangleIndexPerMesh[meshId], node->triangleCountPerMesh[meshId]);
        return;
    }

    for (int i = 0; i < 8; i++) {
        if (node->children[i]) RecurseDrawOctree(node->children[i]);
    }
}

Camera raylib_camera = { 0 };

void checkFrustumOctree(OctreeNode* node)
{
    if (!node) return;

    node->visible = CheckCollisionBoxSphere(node->box, raylib_camera.position, 1.0f);

    if (node->visible == false || node->isLeaf) return;

    for (int i = 0; i < 8; i++) {
        checkFrustumOctree(node->children[i]);
    }

}

void drawOctree(Model mod)
{

    CameraData* camdata = (CameraData*)camera.data;

    raylib_camera = camdata->cam;

    checkFrustumOctree(mapTree.root);


    for (int meshi = 0; meshi < mod.meshCount; meshi++)
    {
        drawingMaterial = mod.materials[mod.meshMaterial[meshi]];
        drawingMesh = mod.meshes + meshi;
        drawingTransform = mod.transform;
        meshId = meshi;
        //DrawMesh(mesh, mat, transform);


        OctreeNode* node = mapTree.root;

        RecurseDrawOctree(node);
        
        
    }
}


void loadSubModelsToResource(model_t* mt)
{

    static int maxvert = 0;

    int beforemark = Hunk_LowMark();

    typedef struct VertPerTexture{
        int tex_idx;
        int mesh_idx;
        int num_vertices;
        int num_triangles;
        struct VertPerTexture* next;
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

        VertPerTexture* vpts = (VertPerTexture*)Hunk_Alloc(sizeof(VertPerTexture));
        VertPerTexture* curr = NULL;
        VertPerTexture* prev = NULL;

        vpts->tex_idx = -1;
        vpts->tex_idx = -1;

        int numusedtextures = 1;

        int putted = 0;

        for (i = 0, surface = mt->data_surfaces+sm->firstface; i < sm->numfaces; i++, surface++) {

            if (!surface->num_vertices) continue;
            curr = vpts;
            putted = 0;
            while (!putted && curr) {
                if (curr->tex_idx == -1) {
                    curr->tex_idx = surface->tex_idx;
                }
                if (curr->tex_idx == surface->tex_idx) {
                    if (curr->num_vertices + surface->num_vertices < MAX_UNSIGNED_SHORT) {
                        curr->num_vertices += surface->num_vertices;
                        curr->num_triangles += surface->num_triangles;
                        putted = 1;
                        break;
                    }
                }
                prev = curr;
                curr = prev->next;
                
            }

            if (!putted) {
                prev->next = (VertPerTexture*)Hunk_Alloc(sizeof(VertPerTexture));
                prev->next->tex_idx = surface->tex_idx;
                prev->next->mesh_idx = numusedtextures;
                prev->next->num_vertices += surface->num_vertices;
                prev->next->num_triangles += surface->num_triangles;
                numusedtextures++;
            }
            

        }

        Mesh* meshes = (Mesh*)RL_CALLOC(numusedtextures, sizeof(Mesh));   

        md.meshCount = numusedtextures;
        md.meshes = meshes;
        md.materialCount = numusedtextures;
        md.materials = (Material*)RL_CALLOC(numusedtextures, sizeof(Material));
        md.meshMaterial = (int*)RL_CALLOC(numusedtextures, sizeof(int));

        int j = 0;
        for (i = 0; i < numusedtextures; i++) {
            curr = vpts;
            while (curr)
            {
                if (curr->mesh_idx == i) {
                    break;
                }
                curr = curr->next;
            }

            if (!curr) {
                TraceLog(LOG_ERROR, __FILE__ ": " __FUNCTION__ ": curr vertexperface was null");
            }
            
            meshes[i].indices = (unsigned short*)Z_Malloc(curr->num_triangles*3*sizeof(unsigned short));
            meshes[i].vertices = (float*)Hunk_AllocNoFill(curr->num_vertices*3*sizeof(float));
            meshes[i].texcoords = (float*)Hunk_AllocNoFill(curr->num_vertices*2*sizeof(float));
            meshes[i].texcoords2 = (float*)Hunk_AllocNoFill(curr->num_vertices*2*sizeof(float));
            //meshes[0].vertices = (float*)Hunk_AllocNoFill(numvertices*3*sizeof(float));
            meshes[i].vertexCount = 0;
            meshes[i].triangleCount = 0;

            md.materials[i] = LoadMaterialDefault();
    
            md.meshMaterial[i] = i;

            if (mt->data_textures[curr->tex_idx].type == TEXTYPE_SKY) {
                md.materials[i].shader = skybox_shader;
                texkey = loadTextureResource("\\sky");
                md.materials[i].maps[MATERIAL_MAP_CUBEMAP].texture = getTextureResource(&texkey);
            } else {
                md.materials[i].shader = lightmap_shader; 
                texkey = loadTextureResource(mt->data_textures[curr->tex_idx].name);
                md.materials[i].maps[MATERIAL_MAP_ALBEDO].texture = getTextureResource(&texkey);
                texkey = loadTextureResource("\\light");
                md.materials[i].maps[MATERIAL_MAP_METALNESS].texture = getTextureResource(&texkey);
            }
        }

        int firsttri = 0;
        int firstvert = 0;
        int mesh_idx = 0;
        
        for (i = 0, surface = mt->data_surfaces+sm->firstface; i < sm->numfaces; i++, surface++) {
            curr = vpts;
            while (curr)
            {
                if (curr->tex_idx == surface->tex_idx && meshes[curr->mesh_idx].vertexCount + surface->num_vertices < MAX_UNSIGNED_SHORT) {
                    break;
                }
                curr = curr->next;
            }


            mesh_idx = curr->mesh_idx;
            firstvert = meshes[mesh_idx].vertexCount;
            firsttri = meshes[mesh_idx].triangleCount;
            memcpy(meshes[mesh_idx].vertices+firstvert*3, basemesh->vertices + surface->num_firstvertex*3, surface->num_vertices*3*sizeof(float) );
            memcpy(meshes[mesh_idx].texcoords+firstvert*2, basemesh->texcoords + surface->num_firstvertex*2, surface->num_vertices*2*sizeof(float) );
            memcpy(meshes[mesh_idx].texcoords2+firstvert*2, basemesh->texcoords2 + surface->num_firstvertex*2, surface->num_vertices*2*sizeof(float) );


            for (int tri = 0; tri < surface->num_triangles; tri++) {
                meshes[mesh_idx].indices[(firsttri + tri)*3 + 0] = (unsigned short)(firstvert);
                meshes[mesh_idx].indices[(firsttri + tri)*3 + 1] = (unsigned short)(firstvert + tri + 2);
                meshes[mesh_idx].indices[(firsttri + tri)*3 + 2] = (unsigned short)(firstvert + tri + 1);
            }

            if (firstvert >= MAX_UNSIGNED_SHORT) {
                TraceLog(LOG_ERROR, __FILE__ ": " __FUNCTION__ ": Max vertex reached: %i", firstvert);
            }

            surface->tex_idx = mesh_idx;
            surface->num_firsttriangle = firsttri;
            surface->num_firstvertex = meshes[mesh_idx].vertexCount;

            meshes[mesh_idx].vertexCount += surface->num_vertices;
            meshes[mesh_idx].triangleCount += surface->num_triangles;

            
            
        }



        


        for (i = 0;i < numusedtextures; i++) {
            UploadMesh(meshes+i, true);
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


    TraceLog(LOG_INFO, "min %f %f %f : max %f %f %f", mapMod.aabb[0][0],mapMod.aabb[0][1],mapMod.aabb[0][2],mapMod.aabb[1][0],mapMod.aabb[1][1],mapMod.aabb[1][2]);
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

    
	
    lightmap_shader = LoadShader(TextFormat("res/shaders/shader.vs"), TextFormat("res/shaders/shader.fs"));

    discard_shader = LoadShader(TextFormat("res/shaders/shader_discard.vs"), TextFormat("res/shaders/shader_discard.fs"));

    skybox_shader = LoadShader(TextFormat("res/shaders/skybox.vs"),TextFormat("res/shaders/skybox.fs"));
    SetShaderValue(skybox_shader, GetShaderLocation(skybox_shader, "environmentMap"), (int[1]){ MATERIAL_MAP_CUBEMAP }, SHADER_UNIFORM_INT);
    SetShaderValue(skybox_shader, GetShaderLocation(skybox_shader, "doGamma"), (int[1]){ 0 }, SHADER_UNIFORM_INT);
    SetShaderValue(skybox_shader, GetShaderLocation(skybox_shader, "vflipped"), (int[1]){ 0 }, SHADER_UNIFORM_INT);

    Image image = LoadImage("res/skybox.png");
    setTextureResource(LoadTextureCubemap(image, CUBEMAP_LAYOUT_AUTO_DETECT), "\\sky");
    UnloadImage(image);

    beforeMapMark = Hunk_LowMark();
    loadMapResource("qbj3_radiatoryang.bsp");
    

}

void unloadMapResource()
{
    for (int i = 1; i < RESOURCES_SIZE; i++) {
        if (resources[i].occupied && resources[i].usage == MAP_USAGE) {
            switch (resources[i].type) {
                case RES_TEXTURE:
                    UnloadTexture(resources[i].texture);
                    break;
                case RES_MODEL:
                    for (int j = 0; j < resources[i].model.meshCount; j++) {
                        Z_Free(resources[i].model.meshes[j].indices);
                        resources[i].model.meshes[j].indices = NULL;
                    }
                    UnloadModel(resources[i].model);
                    break;
                default:
                    break;
            }
            memset(resources + i, 0, sizeof(Resource));
        }
    }
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
    if (IsShaderValid(skybox_shader)) {
        UnloadShader(skybox_shader);
        skybox_shader = (Shader){0};
    }
    if (IsShaderValid(discard_shader)) {
        UnloadShader(discard_shader);
        discard_shader = (Shader){0};
    }
    if (IsShaderValid(lightmap_shader)) {
        UnloadShader(lightmap_shader);
        lightmap_shader = (Shader){0};
    }

    unloadMapResource();
    Model model = (Model){0};
    Texture texture = (Texture){0};
    for (int i = 0; i < RESOURCES_SIZE; i++) {
        if (resources[i].occupied) {
            switch (resources[i].type)
            {
            case RES_MODEL:
                model = resources[i].model;
                UnloadModel(model);
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

    Hunk_FreeToLowMark(beforeMapMark);

    if (b3World_IsValid(g_worldid)) {
        b3DestroyWorld(g_worldid);
    }

    memset(resources, 0, sizeof(resources));
    

}