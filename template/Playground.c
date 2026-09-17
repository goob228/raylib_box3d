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
#include "G_local.h"
#include "SysCvar.h"


Object objects[MAX_OBJECTS] = {0};

Object camera = {0};

int objectCount = 1;


#define FNV_32_OFFSET 2166136261
#define FNV_32_PRIME 16777619





void pg_update(struct Playground* self, EventHandler* eventhandler);

void pg_render(struct Playground* self, WindowHandler* windowhandler);

void pg_cleanUp(struct Playground* self);

int pg_addObject(struct Playground* self, Vector3 pos, Vector3 scale, Resource_key* texId, Resource_key* modelId, ObjectType type);

int pg_addTexture(struct Playground* self, char const* fileName);

int pg_addModel(struct Playground* self, char const* fileName);



int pg_addObject(struct Playground* self, Vector3 pos, Vector3 scale, Resource_key* texId, Resource_key* modelId, ObjectType type)
{
	int objid = objectCount;
	objects[objid].transform = MatrixIdentity();
	objects[objid].pos = (Vector3){ 0.0f, 0.0f, 0.0f }; 
	objects[objid].rot = QuaternionIdentity(); 
	objects[objid].scale = (Vector3){ 1.0f, 1.0f, 1.0f }; 
	objects[objid].alive = true; 
	objects[objid].type = OBJ_NONE; 
	objects[objid].parent = (Object*)0; 
	objects[objid].physId = 0; 
	objects[objid].onRemove = false;

	objects[objid].update = (&ob_update);
	objects[objid].updateMatrix = (&ob_updateMatrix);
	objects[objid].draw = (&ob_draw);
	objects[objid].setParent = (&ob_setParent);
	objects[objid].scale = scale;
	objects[objid].posCurr.x = pos.x;
	objects[objid].posCurr.y = pos.y;
	objects[objid].posCurr.z = pos.z;
	objects[objid].texres = *texId;
	objects[objid].modelres = *modelId;
	objects[objid].type = type;
	objects[objid].updateMatrix(&objects[objid]);

	/*

	if (type == OBJ_PROP || type == OBJ_OBSTACLE) {
		BoundingBox bb = GetModelBoundingBox(getModelResource(&(objects[objid].modelres)));

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
		objects[objid].physId = self->bodyCount;
		self->bodyCount++;
	} */

	objectCount += 1;

	return objectCount-1;
	
}





b3WorldId g_worldid = {0};

b3Recording* recording = NULL;

void pg_init(struct Playground* self, int targetFPS)
{

	self->render = (&pg_render);
	self->update = (&pg_update);
	self->cleanUp = (&pg_cleanUp);

	for (int i = 0; i < MAX_OBJECTS; i++) {
		objects[i].onRemove = true;
	}

	self->bodyCount = 1;
	objectCount = 1;
	self->springCount = 1;


	b3WorldDef worldDef = b3DefaultWorldDef();
	worldDef.gravity = (b3Vec3){ 0.0f, -10.0f, 0.0f };
	worldDef.enableContinuous = true;

	g_worldid = b3CreateWorld(&worldDef);

	self->worldId = g_worldid;

	gc_init(&camera);


	/*
	Resource_key texkey2 = (Resource_key){ 0 };

	Resource_key modkey2 = (Resource_key){ 0 };
	
	int ti = pg_addObject(self, (Vector3) { 0.0f, 0.0f, 0.0f }, (Vector3) { 1.0f, 1.0f, 1.0f }, &texkey2, &modkey2, OBJ_NONE);

	character_create(&objects[ti], &camera, self);

	camera.setParent(&camera, &objects[ti]);

	Resource_key key =  (Resource_key){0};
	Resource_key modelkey =  loadModelResource("\\map");

	pg_addObject(self, (Vector3){0.0f, 0.0f, 0.0f}, (Vector3){1.0f,1.0f,1.0f}, &key, &modelkey, OBJ_NONE);

	*/
	

	

	

	



	

	

	//recording = b3CreateRecording( 0 );
	//b3World_StartRecording( self->worldId, recording );  

	

}


void pg_update(struct Playground* self, EventHandler* eventhandler)
{
	if (eventhandler)
		self->eh = *eventhandler;
	else 
		self->eh = (EventHandler){0};

	b3World_Step(self->worldId, (float)host_netinterval, 1);

	


	for (int i = 1; i < objectCount; i++) {
		if (objects[i].onRemove != true) {
			objects[i].update(&objects[i],self);
			
		}
	}
}

void pg_camUpdate(struct Playground* self, EventHandler* eventhandler)
{
	if (eventhandler)
		self->eh = *eventhandler;
	else 
		self->eh = (EventHandler){0};
	camera.update((Object*)&camera, self);
}

void pg_render(struct Playground* self, WindowHandler* windowhandler)
{
	((CameraData*)camera.data)->startFrame(&camera);

	if (cv_wireframe.valueb) {
		rlEnableWireMode();
		rlDisableBackfaceCulling();
	}
	

	for (int i = 1; i < objectCount; i++) {
		if (objects[i].onRemove != true) {
			objects[i].draw(&objects[i], self);
		}
	}

	if (cv_wireframe.valueb) {
		rlEnableBackfaceCulling();
		rlDisableWireMode();
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

	

	EndShaderMode();
	((CameraData*)camera.data)->endFrame(&camera);

	Resource_key key = loadTextureResource("\\light");

	Texture tex = getTextureResource(&key);

	DrawTextureEx(tex, (Vector2){0.0f, 0.0f}, 0.0f, (0.25f*1024.0f/(float)tex.width), WHITE);

}


void pg_cleanUp(struct Playground* self)
{

	//b3World_StopRecording( self->worldId );
	
	//b3SaveRecordingToFile( recording, "session.b3rec" ); 
	//b3DestroyRecording( recording );
	
	memset(objects, 0, sizeof(objects));

}