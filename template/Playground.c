#include "Playground.h"

#include <malloc.h>


#include <raylib.h>
#include <rlgl.h>


#include "WindowHandler.h"
#include "EventHandler.h"
#include "Camera.h"
#include "Object.h"
#include "MapLoader.h"

#include "Prefabs.h"


void pg_update(struct Playground* self, EventHandler* eventhandler);

void pg_render(struct Playground* self, WindowHandler* windowhandler);

void pg_cleanUp(struct Playground* self);

int pg_addObject(struct Playground* self, Vector3 pos, Vector3 scale, int texId, int modelId, ObjectType type);

int pg_addTexture(struct Playground* self, char const* fileName);

int pg_addModel(struct Playground* self, char const* fileName);



int pg_addObject(struct Playground* self, Vector3 pos, Vector3 scale, int texId, int modelId, ObjectType type)
{
	int objid = self->objCount;
	self->objects[objid].transform = MatrixIdentity();
	self->objects[objid].pos = (Vector3){ 0.0f, 0.0f, 0.0f }; 
	self->objects[objid].rot = QuaternionIdentity(); 
	self->objects[objid].scale = (Vector3){ 1.0f, 1.0f, 1.0f }; 
	self->objects[objid].alive = true; 
	self->objects[objid].type = OBJ_NONE; 
	self->objects[objid].parent = (Object*)0; 
	self->objects[objid].physId = 0; 
	self->objects[objid].texId = 0; 
	self->objects[objid].modelId = 0; 
	self->objects[objid].onRemove = false;

	self->objects[objid].update = (&ob_update);
	self->objects[objid].updateMatrix = (&ob_updateMatrix);
	self->objects[objid].draw = (&ob_draw);
	self->objects[objid].setParent = (&ob_setParent);
	self->objects[objid].scale = scale;
	self->objects[objid].pos = pos;
	self->objects[objid].texId = texId;
	self->objects[objid].modelId = modelId;
	self->objects[objid].type = type;
	self->objects[objid].updateMatrix(&self->objects[objid]);

	if (type == OBJ_PROP || type == OBJ_OBSTACLE) {
		BoundingBox bb = GetModelBoundingBox(self->models[modelId]);

		b3Transform transform = { 0 };
		transform.p.x = (bb.max.x + bb.min.x) * scale.x / 2.0f;
		transform.p.y = (bb.max.y + bb.min.y) * scale.y / 2.0f;
		transform.p.z = (bb.max.z + bb.min.z) * scale.z / 2.0f;
		Quaternion q = QuaternionIdentity();
		transform.q.v.x = q.x;
		transform.q.v.y = q.y;
		transform.q.v.z = q.z;
		transform.q.s = q.w;
		
		b3BodyDef bodyDef = b3DefaultBodyDef();
		if (type == OBJ_PROP)
			bodyDef.type = b3_dynamicBody;
		bodyDef.position = (b3Vec3){ pos.x, pos.y, pos.z };
		b3BodyId bodyId = b3CreateBody(self->worldId, &bodyDef);

		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.density = 30.0f;
		shapeDef.baseMaterial.friction = 0.5f;
		
		b3BoxHull dynamicBox = b3MakeBoxHull(	(bb.max.x - bb.min.x) * scale.x / 2.0f,
												(bb.max.y - bb.min.y) * scale.y / 2.0f,
												(bb.max.z - bb.min.z) * scale.z / 2.0f);

		

		b3CreateTransformedHullShape(bodyId, &shapeDef, &dynamicBox.base, transform, (b3Vec3){1.0f,1.0f,1.0f});
		
	


		self->bodies[self->bodyCount] = bodyId;
		self->objects[objid].physId = self->bodyCount;
		self->bodyCount++;
	}

	self->objCount += 1;

	return self->objCount-1;
	
}



int pg_addTexture(struct Playground* self, char const * fileName)
{
	self->textures[self->textureCount] = LoadTexture(fileName);
	self->textureCount++;

	return 0;
}

int pg_addModel(struct Playground* self, char const* fileName)
{
	self->models[self->modelCount] = LoadModel(fileName);
	self->models[self->modelCount].materials[0].shader = self->basicShader;
	self->modelCount++;

	return 0;
}

Model LoadModelFromModel_t(Playground* self, model_t* mt)
{
	int numofmeshes = 0;

	for (int i = 0; i < mt->meshCount; i++) {
		if (mt->mesh[i].vertices && mt->mesh[i].vertexCount) {
			numofmeshes++;
		}
	}

	Mesh* meshes = (Mesh*)RL_CALLOC(numofmeshes, sizeof(Mesh));


	for (int i = 0, j = 0; i < mt->meshCount; i++) {
		if (mt->mesh[i].vertices && mt->mesh[i].vertexCount) {
			meshes[j].triangleCount = 		mt->mesh[i].triangleCount;
			meshes[j].vertexCount = 		mt->mesh[i].vertexCount;
			meshes[j].vertices =			mt->mesh[i].vertices;
			meshes[j].normals =			mt->mesh[i].normals;
			meshes[j].texcoords =			mt->mesh[i].texcoords;
			UploadMesh(&meshes[j], false);
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
	for (int i = 0; i < model.materialCount; i++) {
		model.materials[i] = LoadMaterialDefault();
		texid = i%self->textureCount;
		if (!texid) texid++;
		model.materials[i].maps[MATERIAL_MAP_ALBEDO].texture = self->textures[texid];
	}

    return model;
}

b3Recording* recording = NULL;

void pg_init(struct Playground* self, int targetFPS)
{

	self->addModel = (&pg_addModel);
	self->addTexture = (&pg_addTexture);
	self->addObject = (&pg_addObject);
	self->render = (&pg_render);
	self->update = (&pg_update);
	self->cleanUp = (&pg_cleanUp);

	for (int i = 0; i < MAX_SPRINGS; i++) {
		self->objects[i].onRemove = true;
	}

	self->bodyCount = 1;
	self->objCount = 1;
	self->textureCount = 1;
	self->modelCount = 1;
	self->springCount = 1;

	self->elapsed = 0.0f;


	self->targetFPS = targetFPS;
	self->targetDeltaTime = 1.0f / (float)self->targetFPS;



	gc_init(&self->camera);


	b3WorldDef worldDef = b3DefaultWorldDef();
	worldDef.gravity = (b3Vec3){ 0.0f, -10.0f, 0.0f };
	worldDef.enableContinuous = true;

	self->worldId = b3CreateWorld(&worldDef);


	self->basicShader = LoadShader(0, "");

	int ti = self->objCount;
	self->objects[ti].transform = MatrixIdentity();
	self->objects[ti].rot = QuaternionIdentity();
	self->objects[ti].pos = (Vector3){0.0f, 10.0f, 0.0f};
	self->objects[ti].scale = (Vector3){ 1.0f, 1.0f, 1.0f }; 
	self->objects[ti].alive = true; 
	self->objects[ti].type = OBJ_NONE; 
	self->objects[ti].parent = (Object*)0; 
	self->objects[ti].physId = 0; 
	self->objects[ti].texId = 0; 
	self->objects[ti].modelId = 0; 
	self->objects[ti].onRemove = false;

	self->objects[ti].update = (&ob_update);
	self->objects[ti].updateMatrix = (&ob_updateMatrix);
	self->objects[ti].draw = (&ob_draw);
	self->objects[ti].setParent = (&ob_setParent);
	self->objects[ti].texId = 2;
	
	self->models[self->modelCount] = LoadModelFromMesh(GenMeshCylinder(0.2f, 0.5f, 8));
	

	self->objects[ti].modelId = self->modelCount;
	self->objects[ti].type = OBJ_OBSTACLE;
	self->objects[ti].updateMatrix(&self->objects[ti]);

	character_create(&self->objects[ti], &self->camera, self);

	self->camera.setParent(&self->camera, &self->objects[ti]);
	self->modelCount++;
	self->objCount++;
	
	pg_addTexture(self, "res/Bricks_06-128x128.png");
	pg_addTexture(self, "res/Wood_17-128x128.png");
	pg_addTexture(self, "res/car2.png");
	pg_addTexture(self, "res/car6.png");
	

	model_t* mapMod = loadMyMap("res/test2.bsp");

	//mapMesh->indices = NULL;

	
	self->models[self->modelCount] = LoadModelFromModel_t(self, mapMod);


	self->modelCount++;

	pg_addObject(self, (Vector3){0.0f, 0.0f, 0.0f}, (Vector3){1.0f,1.0f,1.0f}, 0, self->modelCount-1, OBJ_NONE);

	
	b3MeshDef def = {0};
	def.vertices      = (b3Vec3*)mapMod->vertices;
	def.vertexCount   = mapMod->vertexCount;
	def.indices       = mapMod->indices;
	def.triangleCount = mapMod->triangleCount;
	def.weldVertices  = true;
	def.identifyEdges = true;            // adjacency info for smooth inter-triangle normals
	def.weldTolerance = 0.01f;
	
	
	
	b3MeshData* mesh = b3CreateMesh(&def, NULL, 0);

	b3BodyDef bodyDef = b3DefaultBodyDef();
	b3BodyId body = b3CreateBody( self->worldId, &bodyDef );

	b3ShapeDef shapeDef = b3DefaultShapeDef();
	b3SurfaceMaterial materials[3];
	materials[0] = (b3SurfaceMaterial){ 0.6f, 0.0f, 0 };
	materials[1] = (b3SurfaceMaterial){ 0.6f, 1.0f, 1 };
	materials[2] = (b3SurfaceMaterial){ 0.1f, 0.0f, 2 };
	shapeDef.materials = materials;
	shapeDef.materialCount = 3;

	b3CreateMeshShape( body, &shapeDef, mesh, b3Vec3_one );

	//recording = b3CreateRecording( 0 );
	//b3World_StartRecording( self->worldId, recording );  

}

void pg_update(struct Playground* self, EventHandler* eventhandler)
{

	self->eh = *eventhandler;
	self->camera.update((Object*)&self->camera, self);

	b3World_Step(self->worldId, self->targetDeltaTime, 1);

	


	for (int i = 1; i <= self->objCount; i++) {
		if (self->objects[i].onRemove != true) {
			self->objects[i].update(&self->objects[i],self);
			
		}
	}
}

void pg_render(struct Playground* self, WindowHandler* windowhandler)
{
	((CameraData*)self->camera.data)->startFrame(&self->camera);
	BeginShaderMode(self->basicShader);


	for (int i = 1; i <= self->objCount; i++) {
		if (self->objects[i].onRemove != true) {
			self->objects[i].draw(&self->objects[i], self);
		}
	}

	Vector3 spos = (Vector3){ 0 };
	Vector3 epos = (Vector3){ 0 };

	for (int i = 0; i < MAX_LINES/2-1; i++) {
		spos.x = self->lines[i * 2 + 0].x;
		spos.y = self->lines[i * 2 + 0].y;
		spos.z = self->lines[i * 2 + 0].z;

		epos.x = self->lines[i * 2 + 1].x;
		epos.y = self->lines[i * 2 + 1].y;
		epos.z = self->lines[i * 2 + 1].z;
		
		DrawLine3D(spos, epos, MAROON);

	}
	self->elapsed += self->targetDeltaTime;
	

	EndShaderMode();
	((CameraData*)self->camera.data)->endFrame(&self->camera);

}


void pg_cleanUp(struct Playground* self)
{

	//b3World_StopRecording( self->worldId );
	
	//b3SaveRecordingToFile( recording, "session.b3rec" ); 
	//b3DestroyRecording( recording );
	
	for (int i = 0; i < MAX_BODIES; i++) {
		if (b3Body_IsValid(self->bodies[i])) {
			b3DestroyBody(self->bodies[i]);
		}
	}

	for (int i = 0; i < MAX_TEXTURES; i++) {
		if (self->textures[i].id != rlGetTextureIdDefault()) rlUnloadTexture(self->textures[i].id);
		
	}

	for (int i = 0; i < MAX_MODELS; i++) {
		if (self->models[i].meshCount > 0) {
			self->models[i].materials[0].shader = (Shader){ 0 };
			self->models[i].materials[0].maps[MATERIAL_MAP_ALBEDO].texture.id = rlGetTextureIdDefault();
			UnloadMaterial(self->models[i].materials[0]);
			self->models[i].materials[0].maps = NULL;
			UnloadModel(self->models[i]);
		}
	}

	UnloadShader(self->basicShader);

}