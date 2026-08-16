#include "Playground.h"

#include <raylib.h>
#include <rlgl.h>


#include "WindowHandler.h"
#include "EventHandler.h"
#include "Camera.h"
#include "Object.h"

#include "Prefabs.h"


void pg_update(struct Playground* self, EventHandler* eventhandler);

void pg_render(struct Playground* self, WindowHandler* windowhandler);

void pg_cleanUp(struct Playground* self);

int pg_addObject(struct Playground* self, Vector3 pos, Vector3 scale, int texId, int modelId, ObjectType type);

int pg_addTexture(struct Playground* self, char const* fileName);

int pg_addModel(struct Playground* self, char const* fileName);



int pg_addObject(struct Playground* self, Vector3 pos, Vector3 scale, int texId, int modelId, ObjectType type)
{
	int objid = self->_objCount;
	self->_objects[objid]._transform = MatrixIdentity();
	self->_objects[objid]._pos = (Vector3){ 0.0f, 0.0f, 0.0f }; 
	self->_objects[objid]._rot = QuaternionIdentity(); 
	self->_objects[objid]._scale = (Vector3){ 1.0f, 1.0f, 1.0f }; 
	self->_objects[objid]._alive = true; 
	self->_objects[objid]._type = OBJ_NONE; 
	self->_objects[objid]._parent = (Object*)0; 
	self->_objects[objid]._physId = 0; 
	self->_objects[objid]._texId = 0; 
	self->_objects[objid]._modelId = 0; 
	self->_objects[objid]._onRemove = false;

	self->_objects[objid].update = (&ob_update);
	self->_objects[objid].updateMatrix = (&ob_updateMatrix);
	self->_objects[objid].draw = (&ob_draw);
	self->_objects[objid].setParent = (&ob_setParent);
	self->_objects[objid]._scale = scale;
	self->_objects[objid]._pos = pos;
	self->_objects[objid]._texId = texId;
	self->_objects[objid]._modelId = modelId;
	self->_objects[objid]._type = type;
	self->_objects[objid].updateMatrix(&self->_objects[objid]);

	if (type == OBJ_PROP || type == OBJ_OBSTACLE) {
		BoundingBox bb = GetModelBoundingBox(self->_models[modelId]);

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
		b3BodyId bodyId = b3CreateBody(self->_worldId, &bodyDef);

		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.density = 30.0f;
		shapeDef.baseMaterial.friction = 0.5f;
		
		b3BoxHull dynamicBox = b3MakeBoxHull(	(bb.max.x - bb.min.x) * scale.x / 2.0f,
												(bb.max.y - bb.min.y) * scale.y / 2.0f,
												(bb.max.z - bb.min.z) * scale.z / 2.0f);

		

		b3CreateTransformedHullShape(bodyId, &shapeDef, &dynamicBox.base, transform, (b3Vec3){1.0f,1.0f,1.0f});
		
	


		self->_bodies[self->_bodyCount] = bodyId;
		self->_objects[objid]._physId = self->_bodyCount;
		self->_bodyCount++;
	}

	self->_objCount += 1;

	return self->_objCount-1;
	
}



int pg_addTexture(struct Playground* self, char const * fileName)
{
	self->_textures[self->_textureCount] = LoadTexture(fileName);
	self->_textureCount++;

	return 0;
}

int pg_addModel(struct Playground* self, char const* fileName)
{
	self->_models[self->_modelCount] = LoadModel(fileName);
	self->_models[self->_modelCount].materials[0].shader = self->_basicShader;
	self->_modelCount++;

	return 0;
}



void pg_init(struct Playground* self, int targetFPS)
{

	self->addModel = (&pg_addModel);
	self->addTexture = (&pg_addTexture);
	self->addObject = (&pg_addObject);
	self->render = (&pg_render);
	self->update = (&pg_update);
	self->cleanUp = (&pg_cleanUp);

	for (int i = 0; i < MAX_SPRINGS; i++) {
		self->_objects[i]._onRemove = true;
	}

	self->_bodyCount = 1;
	self->_objCount = 1;
	self->_textureCount = 1;
	self->_modelCount = 1;
	self->_springCount = 1;

	self->_elapsed = 0.0f;


	self->_targetFPS = targetFPS;
	self->_targetDeltaTime = 1.0f / (float)self->_targetFPS;



	gc_init(&self->_camera);


	b3WorldDef worldDef = b3DefaultWorldDef();
	worldDef.gravity = (b3Vec3){ 0.0f, -10.0f, 0.0f };

	self->_worldId = b3CreateWorld(&worldDef);


	self->_basicShader = LoadShader(0, "");

	int ti = self->_objCount;
	self->_objects[ti]._transform = MatrixIdentity();
	self->_objects[ti]._rot = QuaternionIdentity();
	self->_objects[ti]._pos = (Vector3){0.0f, 10.0f, 0.0f};
	self->_objects[ti]._scale = (Vector3){ 1.0f, 1.0f, 1.0f }; 
	self->_objects[ti]._alive = true; 
	self->_objects[ti]._type = OBJ_NONE; 
	self->_objects[ti]._parent = (Object*)0; 
	self->_objects[ti]._physId = 0; 
	self->_objects[ti]._texId = 0; 
	self->_objects[ti]._modelId = 0; 
	self->_objects[ti]._onRemove = false;

	self->_objects[ti].update = (&ob_update);
	self->_objects[ti].updateMatrix = (&ob_updateMatrix);
	self->_objects[ti].draw = (&ob_draw);
	self->_objects[ti].setParent = (&ob_setParent);
	self->_objects[ti]._texId = 2;
	
	self->_models[self->_modelCount] = LoadModelFromMesh(GenMeshCylinder(0.5f, 2.0f, 8));
	

	self->_objects[ti]._modelId = self->_modelCount;
	self->_objects[ti]._type = OBJ_OBSTACLE;
	self->_objects[ti].updateMatrix(&self->_objects[ti]);

	character_create(&self->_objects[ti], &self->_camera, self);

	self->_camera.setParent(&self->_camera, &self->_objects[ti]);
	self->_modelCount++;
	self->_objCount++;


}

void pg_update(struct Playground* self, EventHandler* eventhandler)
{

	self->eh = *eventhandler;
	self->_camera.update((Object*)&self->_camera, self);

	b3World_Step(self->_worldId, self->_targetDeltaTime, 1);

	


	for (int i = 1; i <= self->_objCount; i++) {
		if (self->_objects[i]._onRemove != true) {
			self->_objects[i].update(&self->_objects[i],self);
			
		}
	}
}

void pg_render(struct Playground* self, WindowHandler* windowhandler)
{
	((CameraData*)self->_camera.data)->startFrame(&self->_camera);
	BeginShaderMode(self->_basicShader);


	for (int i = 1; i <= self->_objCount; i++) {
		if (self->_objects[i]._onRemove != true) {
			self->_objects[i].draw(&self->_objects[i], self);
		}
	}

	Vector3 spos = (Vector3){ 0 };
	Vector3 epos = (Vector3){ 0 };

	for (int i = 0; i < MAX_LINES/2-1; i++) {
		spos.x = self->_lines[i * 2 + 0].x;
		spos.y = self->_lines[i * 2 + 0].y;
		spos.z = self->_lines[i * 2 + 0].z;

		epos.x = self->_lines[i * 2 + 1].x;
		epos.y = self->_lines[i * 2 + 1].y;
		epos.z = self->_lines[i * 2 + 1].z;
		
		DrawLine3D(spos, epos, MAROON);

	}
	self->_elapsed += self->_targetDeltaTime;
	

	EndShaderMode();
	((CameraData*)self->_camera.data)->endFrame(&self->_camera);

}


void pg_cleanUp(struct Playground* self)
{
	
	for (int i = 0; i < MAX_BODIES; i++) {
		if (b3Body_IsValid(self->_bodies[i])) {
			b3DestroyBody(self->_bodies[i]);
		}
	}

	for (int i = 0; i < MAX_TEXTURES; i++) {
		if (self->_textures[i].id != rlGetTextureIdDefault()) rlUnloadTexture(self->_textures[i].id);
		
	}

	for (int i = 0; i < MAX_MODELS; i++) {
		if (self->_models[i].meshCount > 0) {
			self->_models[i].materials[0].shader = (Shader){ 0 };
			self->_models[i].materials[0].maps[MATERIAL_MAP_ALBEDO].texture.id = rlGetTextureIdDefault();
			UnloadMaterial(self->_models[i].materials[0]);
			self->_models[i].materials[0].maps = NULL;
			UnloadModel(self->_models[i]);
		}
	}

	UnloadShader(self->_basicShader);

}