#include "Playground.h"

#include <raylib.h>
#include <rlgl.h>


#include "WindowHandler.h"
#include "EventHandler.h"
#include "Camera.h"
#include "Object.h"

#include "Prefabs.h"

#include "malloc.h"

#include "stdio.h"



int pg_addObjectPointer(struct Playground* self, Object* object)
{
	for (int i = 1; i < self->_objCount; i++) {
		if (self->_objects[i] == 0) {
			self->_objects[i] = object;
			return i;
		}
	}
	self->_objects[self->_objCount] = object;
	self->_objCount += 1;

	return self->_objCount - 1;
}

int pg_addObject(struct Playground* self, Vector3 pos, Vector3 scale, int texId, int modelId, ObjectType type)
{
	self->_objects[self->_objCount] = (Object*)malloc(sizeof(Object));
	self->_objects[self->_objCount]->_transform = MatrixIdentity();
	self->_objects[self->_objCount]->_pos = (Vector3){ 0.0f, 0.0f, 0.0f }; 
	self->_objects[self->_objCount]->_rot = QuaternionIdentity(); 
	self->_objects[self->_objCount]->_scale = (Vector3){ 1.0f, 1.0f, 1.0f }; 
	self->_objects[self->_objCount]->_alive = true; 
	self->_objects[self->_objCount]->_type = OBJ_NONE; 
	self->_objects[self->_objCount]->_parent = (Object*)0; 
	self->_objects[self->_objCount]->_physId = 0; 
	self->_objects[self->_objCount]->_texId = 0; 
	self->_objects[self->_objCount]->_modelId = 0; 
	self->_objects[self->_objCount]->_onRemove = false;

	self->_objects[self->_objCount]->update = (&ob_update);
	self->_objects[self->_objCount]->updateMatrix = (&ob_updateMatrix);
	self->_objects[self->_objCount]->draw = (&ob_draw);
	self->_objects[self->_objCount]->setParent = (&ob_setParent);
	self->_objects[self->_objCount]->_scale = scale;
	self->_objects[self->_objCount]->_pos = pos;
	self->_objects[self->_objCount]->_texId = texId;
	self->_objects[self->_objCount]->_modelId = modelId;
	self->_objects[self->_objCount]->_type = type;
	self->_objects[self->_objCount]->updateMatrix(self->_objects[self->_objCount]);

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

		b3BoxHull dynamicBox = b3MakeBoxHull(	(bb.max.x - bb.min.x) * scale.x / 2.0f,
												(bb.max.y - bb.min.y) * scale.y / 2.0f,
												(bb.max.z - bb.min.z) * scale.z / 2.0f);

		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.density = 50.0f;
		shapeDef.baseMaterial.friction = 0.0f;

		b3CreateTransformedHullShape(bodyId, &shapeDef, &dynamicBox.base, transform, (b3Vec3){1.0f,1.0f,1.0f});
		self->_bodies[self->_bodyCount] = bodyId;
		self->_objects[self->_objCount]->_physId = self->_bodyCount;
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
	for (int i = 0; i < MAX_SPRINGS; i++) {
		self->_objects[i] = (Object*)0;
	}

	self->_bodyCount = 1;
	self->_objCount = 1;
	self->_textureCount = 1;
	self->_modelCount = 1;
	self->_springCount = 1;

	self->_elapsed = 0.0f;


	self->_targetFPS = targetFPS;
	self->_targetDeltaTime = 1.0f / (float)self->_targetFPS;

#ifndef PLATFORM_WEB
	if (ChangeDirectory("D:/Github/raylib_box3d/template/"))
		TraceLog(LOG_ERROR, "Failed to set custom directory: %s", GetWorkingDirectory()); // FIXME TODO HACK
#endif

	gc_init(&self->_camera);


	b3WorldDef worldDef = b3DefaultWorldDef();
	worldDef.gravity = (b3Vec3){ 0.0f, -10.0f, 0.0f };

	self->_worldId = b3CreateWorld(&worldDef);


	self->_basicShader = LoadShader(0, "");





}

void pg_update(struct Playground* self, EventHandler* eventhandler)
{

	self->_keys = eventhandler->_keys;
	self->_pressedKeys = eventhandler->_pressedKeys;
	self->_mx = eventhandler->_mx;
	self->_my = eventhandler->_my;
	self->_camera.update((Object*)&self->_camera, self);

	b3World_Step(self->_worldId, self->_targetDeltaTime, 2);

	


	for (int i = 1; i <= self->_objCount; i++) {
		if (self->_objects[i] != 0) {
			self->_objects[i]->update(self->_objects[i],self);
			/*
			if (self->_objects[i]->_onRemove) {
				free(self->_objects[i]);
				self->_objects[i] = 0;
			}
			*/
			
		}
	}
}

void pg_render(struct Playground* self, WindowHandler* windowhandler)
{
	self->_camera.startFrame(&self->_camera);
	BeginShaderMode(self->_basicShader);


	for (int i = 1; i <= self->_objCount; i++) {
		if (self->_objects[i] != 0) {
			self->_objects[i]->draw(self->_objects[i], self);
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
	self->_camera.endFrame(&self->_camera);

}


void pg_cleanUp(struct Playground* self)
{

	for (int i = 0; i < MAX_OBJECTS; i++) {
		if (self->_objects[i] != 0) {
			//_objects[i]->unLoad();
			free(self->_objects[i]);
			self->_objects[i] = 0;
		}
	}
	
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