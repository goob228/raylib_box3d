#ifndef PLAYGROUND_H
#define PLAYGROUND_H








#include "EventHandler.h"
#include "WindowHandler.h"
#include "Camera.h"
#include "Object.h"
#include "Animation.h"
#include <box3d/box3d.h>



#include "stdint.h"

#define MAX_BODIES 512
#define MAX_OBJECTS 512
#define MAX_TEXTURES 512
#define MAX_MODELS 512
#define MAX_SPRINGS 512

#define MAX_LINES 64


typedef struct Playground
{
	

	int (*addObject)(struct Playground* self, Vector3 pos, Vector3 scale, int texId, int modelId, ObjectType type);
	int (*addTexture)(struct Playground* self, char const* fileName);
	int (*addModel)(struct Playground* self, char const* fileName);



	void (*init)(struct Playground* self, int targetFPS);
	void (*render)(struct Playground* self, WindowHandler* windowhandler);
	void (*cleanUp)(struct Playground* self);
	void (*update)(struct Playground* self, EventHandler* eventhandler);

	b3BodyId bodies[MAX_BODIES];
	Object objects[MAX_OBJECTS];
	Texture2D textures[MAX_TEXTURES];
	Model models[MAX_MODELS];
	Spring springs[MAX_SPRINGS];



	b3Pos lines[MAX_LINES];

	int bodyCount;
	int objCount;
	int textureCount;
	int modelCount;
	int springCount;

	Shader basicShader;

	Object camera;
	b3WorldId worldId;

	int targetFPS;
	float targetDeltaTime;


	EventHandler eh;

	float elapsed;

} Playground;





void pg_init(struct Playground* self, int targetFPS);



#endif