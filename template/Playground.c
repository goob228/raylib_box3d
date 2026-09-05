#include "Playground.h"

#include <malloc.h>
#include <string.h>
#include <stdio.h>

#include <raylib.h>
#include <rlgl.h>



#include "WindowHandler.h"
#include "EventHandler.h"
#include "Camera.h"
#include "Object.h"
#include "MapLoader.h"
#include "Texture.h"
#include "Resource.h"

#include "Prefabs.h"




void pg_update(struct Playground* self, EventHandler* eventhandler);

void pg_render(struct Playground* self, WindowHandler* windowhandler);

void pg_cleanUp(struct Playground* self);

int pg_addObject(struct Playground* self, Vector3 pos, Vector3 scale, Resource_key* texId, Resource_key* modelId, ObjectType type);

int pg_addTexture(struct Playground* self, char const* fileName);

int pg_addModel(struct Playground* self, char const* fileName);



int pg_addObject(struct Playground* self, Vector3 pos, Vector3 scale, Resource_key* texId, Resource_key* modelId, ObjectType type)
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
	self->objects[objid].onRemove = false;

	self->objects[objid].update = (&ob_update);
	self->objects[objid].updateMatrix = (&ob_updateMatrix);
	self->objects[objid].draw = (&ob_draw);
	self->objects[objid].setParent = (&ob_setParent);
	self->objects[objid].scale = scale;
	self->objects[objid].pos = pos;
	self->objects[objid].texres = *texId;
	self->objects[objid].modelres = *modelId;
	self->objects[objid].type = type;
	self->objects[objid].updateMatrix(&self->objects[objid]);

	if (type == OBJ_PROP || type == OBJ_OBSTACLE) {
		BoundingBox bb = GetModelBoundingBox(getModelResource(&(self->objects[objid].modelres)));

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









b3Recording* recording = NULL;

void pg_init(struct Playground* self, int targetFPS)
{

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

	self->worldId = g_worldid;

	gc_init(&self->camera);



	self->basicShader = LoadShader(0, "");

	int ti = self->objCount;
	self->objects[ti].transform = MatrixIdentity();
	self->objects[ti].rot = QuaternionIdentity();
	self->objects[ti].pos = (Vector3){0.0f, 20.0f, -10.0f};
	self->objects[ti].scale = (Vector3){ 1.0f, 1.0f, 1.0f }; 
	self->objects[ti].alive = true; 
	self->objects[ti].type = OBJ_NONE; 
	self->objects[ti].parent = (Object*)0; 
	self->objects[ti].physId = 0; 
	self->objects[ti].onRemove = false;

	self->objects[ti].update = (&ob_update);
	self->objects[ti].updateMatrix = (&ob_updateMatrix);
	self->objects[ti].draw = (&ob_draw);
	self->objects[ti].setParent = (&ob_setParent);
	self->objects[ti].texres = loadTextureResource("Bricks_06");
	

	self->objects[ti].modelres = setModelResource(LoadModelFromMesh(GenMeshCylinder(0.2f, 0.5f, 8)), "cylinder");

	self->objects[ti].type = OBJ_OBSTACLE;
	self->objects[ti].updateMatrix(&self->objects[ti]);
	

	character_create(&self->objects[ti], &self->camera, self);

	self->camera.setParent(&self->camera, &self->objects[ti]);
	self->modelCount++;
	self->objCount++;
	

	

	

	



	Resource_key key =  (Resource_key){0};
	Resource_key modelkey =  loadModelResource("\\map");

	pg_addObject(self, (Vector3){0.0f, 0.0f, 0.0f}, (Vector3){1.0f,1.0f,1.0f}, &key, &modelkey, OBJ_NONE);

	

	//recording = b3CreateRecording( 0 );
	//b3World_StartRecording( self->worldId, recording );  

	

}


void pg_update(struct Playground* self, EventHandler* eventhandler)
{
	if (eventhandler)
		self->eh = *eventhandler;
	else 
		self->eh = (EventHandler){0};
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

	//Resource_key key = loadTextureResource("\\light");

	//DrawTextureRec(getTextureResource(&key), (Rectangle){0, 0, 500, 500}, (Vector2){0, 0}, WHITE);

}


void pg_cleanUp(struct Playground* self)
{

	//b3World_StopRecording( self->worldId );
	
	//b3SaveRecordingToFile( recording, "session.b3rec" ); 
	//b3DestroyRecording( recording );
	

	UnloadShader(self->basicShader);

}