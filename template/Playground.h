#ifndef PLAYGROUND_H
#define PLAYGROUND_H





#include <box3d/box3d.h>


#include "EventHandler.h"
#include "WindowHandler.h"
#include "Camera.h"
#include "Object.h"
#include "Animation.h"
#include "Resource.h"



#include "stdint.h"




#define MAX_BODIES 512
#define MAX_OBJECTS 1024
#define MAX_SPRINGS 128

#define MAX_LINES 64


extern Object objects[MAX_OBJECTS];

extern Object camera;

typedef struct Playground
{
	



	void (*init)(struct Playground* self, int targetFPS);
	void (*render)(struct Playground* self, WindowHandler* windowhandler);
	void (*cleanUp)(struct Playground* self);
	void (*update)(struct Playground* self, EventHandler* eventhandler);

	b3BodyId bodies[MAX_BODIES];
	Spring springs[MAX_SPRINGS];



	b3Pos lines[MAX_LINES];

	int bodyCount;
	int objCount;
	int springCount;


	
	b3WorldId worldId;



	EventHandler eh;

} Playground;


int pg_addObject(struct Playground* self, Vector3 pos, Vector3 scale, Resource_key* texId, Resource_key* modelId, ObjectType type);

void pg_update(struct Playground* self, EventHandler* eventhandler);

void pg_camUpdate(struct Playground* self, EventHandler* eventhandler);

void pg_init(struct Playground* self, int targetFPS);


#endif