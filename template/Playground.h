#ifndef PLAYGROUND_H
#define PLAYGROUND_H

#include <cstdint>

#include <box3d/box3d.h>
#include <lua/lua.hpp>


#include "EventHandler.h"
#include "WindowHandler.h"
#include "Camera.h"
#include "Object.h"

#define MAX_BODIES 512
#define MAX_OBJECTS 512
#define MAX_TEXTURES 512
#define MAX_MODELS 512

#define MAX_LINES 64


class Playground
{
public:


	void add_stat_box();
	void add_dyn_box();
	void add_dyn_box2();
	void add_dyn_sphere();

	int addObject(Vector3 pos, Vector3 scale, int texId, int modelId, ObjectType type);
	void delete_object();

	int addTexture(char const * fileName);

	int addModel(char const* fileName);



	void init(int targetFPS);
	void render(WindowHandler* windowhandler);
	void cleanUp();
	void update(EventHandler* eventhandler);

	b3BodyId _bodies[MAX_BODIES] = { 0 };
	Object* _objects[MAX_OBJECTS] = { nullptr };
	Texture2D _textures[MAX_TEXTURES] = { 0 };
	Model _models[MAX_MODELS] = { 0 };

	b3Pos _lines[MAX_LINES];

	int _bodyCount = 1;
	int _objCount = 1;
	int _textureCount = 1;
	int _modelCount = 1;

	Shader _basicShader = { 0 };

	GameCamera _camera;
	b3WorldId _worldId;

	int _targetFPS;
	float _targetDeltaTime;


	uint16_t _keys;
	uint16_t _pressedKeys;
	float _mx;
	float _my;

};

#endif