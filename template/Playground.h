#ifndef PLAYGROUND_H
#define PLAYGROUND_H

#include <cstdint>

#include <box3d/box3d.h>
#include <lua/lua.hpp>


#include "EventHandler.h"
#include "WindowHandler.h"
#include "Camera.h"
#include "Object.h"
#include "Animation.h"

#define MAX_BODIES 512
#define MAX_OBJECTS 512
#define MAX_TEXTURES 512
#define MAX_MODELS 512
#define MAX_SPRINGS 512

#define MAX_LINES 64


typedef struct Playground
{
	


	int (*addObjectPointer)(struct Playground* self, Object* object);

	int (*addObject)(struct Playground* self, Vector3 pos, Vector3 scale, int texId, int modelId, ObjectType type);

	int (*addTexture)(struct Playground* self, char const* fileName);

	int (*addModel)(struct Playground* self, char const* fileName);



	void (*init)(struct Playground* self, int targetFPS);
	void (*render)(struct Playground* self, WindowHandler* windowhandler);
	void (*cleanUp)(struct Playground* self);
	void (*update)(struct Playground* self, EventHandler* eventhandler);

	b3BodyId _bodies[MAX_BODIES] = { 0 };
	Object* _objects[MAX_OBJECTS] = { nullptr };
	Texture2D _textures[MAX_TEXTURES] = { 0 };
	Model _models[MAX_MODELS] = { 0 };
	Spring _springs[MAX_SPRINGS] = { 0 };



	b3Pos _lines[MAX_LINES];

	int _bodyCount = 1;
	int _objCount = 1;
	int _textureCount = 1;
	int _modelCount = 1;
	int _springCount = 1;

	Shader _basicShader = { 0 };

	GameCamera _camera;
	b3WorldId _worldId;

	int _targetFPS;
	float _targetDeltaTime;


	uint16_t _keys;
	uint16_t _pressedKeys;
	float _mx;
	float _my;

	float _elapsed = 0.0f;

} Playground;


int pg_addObjectPointer(struct Playground* self, Object* object);

int pg_addObject(struct Playground* self, Vector3 pos, Vector3 scale, int texId, int modelId, ObjectType type);

int pg_addTexture(struct Playground* self, char const* fileName);

int pg_addModel(struct Playground* self, char const* fileName);

void pg_init(struct Playground* self, int targetFPS);

void pg_update(struct Playground* self, EventHandler* eventhandler);

void pg_render(struct Playground* self, WindowHandler* windowhandler);

void pg_cleanUp(struct Playground* self);

#endif