#include "Playground.h"

#include <box3d/box3d.h>
#include <raylib.h>
#include <rlgl.h>
#include <lua/lua.hpp>

#include "Helper.h"

#include "WindowHandler.h"
#include "EventHandler.h"
#include "Camera.h"
#include "Object.h"




void Playground::add_stat_box()
{
	b3BodyDef groundBodyDef = b3DefaultBodyDef();
	groundBodyDef.position = b3Vec3{ 0.0f, -10.0f, 0.0f };
	b3BodyId groundId = b3CreateBody(_worldId, &groundBodyDef);

	b3BoxHull groundBox = b3MakeCubeHull(10.0f); //b3MakeBoxHull(50.0f, 10.0f, 50.0f);

	b3ShapeDef groundShapeDef = b3DefaultShapeDef();
	b3CreateHullShape(groundId, &groundShapeDef, &groundBox.base);
	_bodies[_bodyCount] = groundId;
	_bodyCount++;
}

void Playground::add_dyn_box()
{
	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = b3_dynamicBody;
	bodyDef.position = b3Vec3{ 0.0f, 10.0f, 0.0f };
	b3BodyId bodyId = b3CreateBody(_worldId, &bodyDef);

	b3BoxHull dynamicBox = b3MakeCubeHull(1.0f);

	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = 1.0f;
	shapeDef.baseMaterial.friction = 0.3f;

	b3CreateHullShape(bodyId, &shapeDef, &dynamicBox.base);
	_bodies[_bodyCount] = bodyId;
	_bodyCount++;
}

void Playground::add_dyn_box2()
{
	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = b3_dynamicBody;
	bodyDef.position = b3Vec3{ -0.6f, 20.0f, -0.6f };
	b3BodyId bodyId = b3CreateBody(_worldId, &bodyDef);

	b3BoxHull dynamicBox = b3MakeCubeHull(1.0f);

	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = 1.0f;
	shapeDef.baseMaterial.friction = 0.3f;

	b3CreateHullShape(bodyId, &shapeDef, &dynamicBox.base);
	_bodies[_bodyCount] = bodyId;
	_bodyCount++;
}

void Playground::add_dyn_sphere()
{
	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = b3_dynamicBody;
	bodyDef.position = b3Vec3{ 1.1f, 15.0f, 1.1f };
	b3BodyId bodyId = b3CreateBody(_worldId, &bodyDef);

	b3Sphere sphere = { b3Vec3_zero, 0.5f };

	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = 1.0f;
	shapeDef.baseMaterial.friction = 0.3f;

	b3CreateSphereShape(bodyId, &shapeDef, &sphere);
	_bodies[_bodyCount] = bodyId;
	_bodyCount++;
}

int Playground::addObject(Vector3 pos, Vector3 scale, int texId, int modelId, ObjectType type)
{
	_objects[_objCount] = new Object();
	_objects[_objCount]->_scale = scale;
	_objects[_objCount]->_pos = pos;
	_objects[_objCount]->_texId = texId;
	_objects[_objCount]->_modelId = modelId;
	_objects[_objCount]->_type = type;
	_objects[_objCount]->updateMatrix();

	if (type == OBJ_PROP || type == OBJ_OBSTACLE) {
		BoundingBox bb = GetModelBoundingBox(_models[modelId]);

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
		bodyDef.position = b3Vec3{ pos.x, pos.y, pos.z };
		b3BodyId bodyId = b3CreateBody(_worldId, &bodyDef);

		b3BoxHull dynamicBox = b3MakeBoxHull(	(bb.max.x - bb.min.x) * scale.x / 2.0f,
												(bb.max.y - bb.min.y) * scale.y / 2.0f,
												(bb.max.z - bb.min.z) * scale.z / 2.0f);

		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.density = 1.0f;
		shapeDef.baseMaterial.friction = 0.3f;

		b3CreateTransformedHullShape(bodyId, &shapeDef, &dynamicBox.base, transform, b3Vec3{1.0f,1.0f,1.0f});
		_bodies[_bodyCount] = bodyId;
		_objects[_objCount]->_physId = _bodyCount;
		_bodyCount++;
	}

	_objCount += 1;

	return _objCount-1;
	
}

void Playground::delete_object()
{
	int i = _objCount-1;
	if (_objects[i] != nullptr) {
		int e = _objects[i]->_physId;
		if (b3Body_IsValid(_bodies[e])) {
			b3DestroyBody(_bodies[e]);
		}

		_bodyCount--;

		delete _objects[i];
		_objects[i] = nullptr;

		
	}
	_objCount--;
}

int Playground::addTexture(char const * fileName)
{
	_textures[_textureCount] = LoadTexture(fileName);
	_textureCount++;

	return 0;
}

int Playground::addModel(char const* fileName)
{
	_models[_modelCount] = LoadModel(fileName);
	_models[_modelCount].materials[0].shader = _basicShader;
	_modelCount++;

	return 0;
}




int Playground::lua_addTexture(lua_State* L)
{
	Playground* pg = (Playground*)lua_touserdata(L, 1);
	
	pg->addTexture(lua_tostring(L, 2));
	int idx = pg->_textureCount - 1;
	lua_pushnumber(L, idx);

	return 1;
}

int Playground::lua_addModel(lua_State* L)
{
	Playground* pg = (Playground*)lua_touserdata(L, 1);

	pg->addModel(lua_tostring(L, 2));

	int idx = pg->_modelCount - 1;
	lua_pushnumber(L, idx);

	return 1;
}

int Playground::lua_addObject(lua_State* L)
{
	Playground* pg = (Playground*)lua_touserdata(L, 1);

	int texId = lua_tonumber(L, 2);
	int modelId = lua_tonumber(L, 3);
	ObjectType type = (ObjectType)(int)lua_tonumber(L, 4);
	float px = (float)lua_tonumber(L, 5);
	float py = (float)lua_tonumber(L, 6);
	float pz = (float)lua_tonumber(L, 7);
	float sx = (float)lua_tonumber(L, 8);
	float sy = (float)lua_tonumber(L, 9);
	float sz = (float)lua_tonumber(L, 10);

	pg->addObject(Vector3{ px, py, pz}, Vector3{ sx, sy, sz}, texId, modelId, type);

	return 1;
}


void Playground::parseLua()
{
	char comand[] = "a = 7 + 11";
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);


	lua_pushlightuserdata(L, this);
	lua_setglobal(L, "pg");

	lua_register(L, "addTexture", lua_addTexture);
	lua_register(L, "addModel", lua_addModel);
	lua_register(L, "addObject", lua_addObject);

	int r = luaL_dofile(L, "D:/Github/raylib_box3d/template/res/textures.lua");

	if (r != LUA_OK){
		TraceLog(LOG_ERROR, "Lua failed: %s", lua_tostring(L, -1));
	}

	lua_close(L);

}

void Playground::init(int targetFPS)
{

	_targetFPS = targetFPS;
	_targetDeltaTime = 1.0f / (float)_targetFPS;

	if (ChangeDirectory("D:/Github/raylib_box3d/template/"))
		TraceLog(LOG_ERROR, "Failed to set custom directory: %s", GetWorkingDirectory()); // FIXME TODO HACK

	_camera.init();


	b3WorldDef worldDef = b3DefaultWorldDef();
	worldDef.gravity = b3Vec3{ 0.0f, -10.0f, 0.0f };

	_worldId = b3CreateWorld(&worldDef);

	
	add_dyn_sphere();
	_basicShader = LoadShader(0, TextFormat("D:/GitHub/raylib/examples/shaders/resources/shaders/glsl330/tiling.fs"));
	float tiling[2] = { 1.0f, 1.0f };
	SetShaderValue(_basicShader, GetShaderLocation(_basicShader, "tiling"), tiling, SHADER_UNIFORM_VEC2);


	parseLua();

}

void Playground::update(EventHandler* eventhandler)
{
	_camera.update();

	b3World_Step(_worldId, _targetDeltaTime, 2);


	for (int i = 1; i <= _objCount; i++) {
		if (_objects[i] != nullptr) {
			_objects[i]->update(this);
		}
	}
}

void Playground::render(WindowHandler* windowhandler)
{
	_camera.startFrame();
	BeginShaderMode(_basicShader);
	//DrawPlane(Vector3{ 0.0f, 0.0f, 0.0f }, Vector2{ 32.0f, 32.0f }, LIGHTGRAY); // Draw ground
	//DrawCube(Vector3{ -16.0f, 2.5f, 0.0f }, 1.0f, 5.0f, 32.0f, BLUE);     // Draw a blue wall
	//DrawCube(Vector3{ 16.0f, 2.5f, 0.0f }, 1.0f, 5.0f, 32.0f, LIME);      // Draw a green wall
	//DrawCube(Vector3{ 0.0f, 2.5f, 16.0f }, 32.0f, 5.0f, 1.0f, GOLD);

	/*
	{
		b3Vec3 position = b3Body_GetPosition(_bodies[2]);
		b3Quat rotation = b3Body_GetRotation(_bodies[2]);

		windowhandler->drawBox(position.x, position.y, position.z, 2.0f);
	}
	*/

	{
		b3Vec3 position = b3Body_GetPosition(_bodies[1]);
		b3Quat rotation = b3Body_GetRotation(_bodies[1]);

		windowhandler->drawSphere(position.x, position.y, position.z, 0.5f);
	}

	for (int i = 1; i <= _objCount; i++) {
		if (_objects[i] != nullptr) {
			_objects[i]->draw(this);
		}
	}



	EndShaderMode();
	_camera.endFrame();

}


void Playground::cleanUp()
{
	for (int i = 0; i < MAX_OBJECTS; i++) {
		if (_objects[i] != nullptr) {
			//_objects[i]->unLoad();
			delete _objects[i];
			_objects[i] = nullptr;
		}
	}
	
	for (int i = 0; i < MAX_BODIES; i++) {
		if (b3Body_IsValid(_bodies[i])) {
			b3DestroyBody(_bodies[i]);
		}
	}

	for (int i = 0; i < MAX_TEXTURES; i++) {
		if (_textures[i].id != rlGetTextureIdDefault()) rlUnloadTexture(_textures[i].id);
		
	}

	for (int i = 0; i < MAX_MODELS; i++) {
		if (_models[i].meshCount > 0) {
			_models[i].materials[0].shader = Shader{ 0 };
			_models[i].materials[0].maps[MATERIAL_MAP_ALBEDO].texture.id = rlGetTextureIdDefault();
			UnloadMaterial(_models[i].materials[0]);
			_models[i].materials[0].maps = NULL;
			UnloadModel(_models[i]);
		}
	}

	UnloadShader(_basicShader);

}