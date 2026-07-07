#ifndef PLAYGROUND_H
#define PLAYGROUND_H


#include <box3d/box3d.h>

#include "EventHandler.h"
#include "WindowHandler.h"
#include "Camera.h"
#include "Object.h"

#define MAX_BODIES 512
#define MAX_OBJECTS 512
#define MAX_TEXTURES 512
#define MAX_MODELS 512

#define MSA_MAX_NAME_LEN 32



typedef enum ParseToken {
	TOK_NONE,
	TOK_END,
	TOK_ADD_TEXTURE,
	TOK_ADD_MODEL,
	TOK_SET_TEXTURE,
	TOK_SET_MODEL,
	TOK_ADD_OBJECT,
	TOK_SET_OBJ_TYPE,
	TOK_END_PARSE,
	TOK_LEN
} ParseToken;




class Playground
{
public:


	void add_stat_box();
	void add_dyn_box();
	void add_dyn_box2();
	void add_dyn_sphere();

	int add_object(Vector3 pos, Vector3 scale, int texId, int modelId, ObjectType type);
	void delete_object();

	int addTexture(char const * fileName);

	int addModel(char const* fileName);

	void reserveNames();
	int findIndex(char* name, char names_array[][MSA_MAX_NAME_LEN], int start, int len);
	void parse(char* txt);

	void init(int targetFPS);
	void render(WindowHandler* windowhandler);
	void cleanUp();
	void update(EventHandler* eventhandler);

	b3BodyId _bodies[MAX_BODIES] = { 0 };
	Object* _objects[MAX_OBJECTS] = { nullptr };
	Texture2D _textures[MAX_TEXTURES] = { 0 };
	Model _models[MAX_MODELS] = { 0 };

	char _namesTokens[TOK_LEN][MSA_MAX_NAME_LEN] = { 0 };
	char _namesTextures[MAX_TEXTURES][MSA_MAX_NAME_LEN] = { 0 };
	char _namesModels[MAX_MODELS][MSA_MAX_NAME_LEN] = { 0 };

	int _bodyCount = 1;
	int _objCount = 1;
	int _textureCount = 1;
	int _modelCount = 1;

	Shader _basicShader = { 0 };

	GameCamera _camera;
	b3WorldId _worldId;

	int _targetFPS;
	float _targetDeltaTime;

};

#endif