#include "LuaBind.h"

#include <lua/lua.hpp>
#include <raylib.h>

#include "Playground.h"
#include "EventHandler.h"
#include "Prefabs.h"





int lua_addTexture(lua_State* L)
{
	Playground* pg = (Playground*)lua_touserdata(L, 1);

	pg->addTexture(lua_tostring(L, 2));
	int idx = pg->_textureCount - 1;
	lua_pushnumber(L, idx);

	return 1;
}

int lua_addModel(lua_State* L)
{
	Playground* pg = (Playground*)lua_touserdata(L, 1);

	pg->addModel(lua_tostring(L, 2));

	int idx = pg->_modelCount - 1;
	lua_pushnumber(L, idx);

	return 1;
}

int lua_addObject(lua_State* L)
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

	int idx = pg->addObject(Vector3{ px, py, pz }, Vector3{ sx, sy, sz }, texId, modelId, type);

	lua_pushnumber(L, idx);

	return 1;
}

int lua_convertToCar(lua_State* L)
{
	Playground* pg = (Playground*)lua_touserdata(L, 1);

	int objid = lua_tonumber(L, 2);

	pg->_objects[objid] = Car::create(pg->_objects[objid], pg);

	return 1;
}

int lua_getObjectPointer(lua_State* L)
{
	Playground* pg = (Playground*)lua_touserdata(L, 1);

	int objid = lua_tonumber(L, 2);

	lua_pushlightuserdata(L, pg->_objects[objid]);

	return 1;
}

int lua_setDamping(lua_State* L)
{
	Car* car = (Car*)lua_touserdata(L, 1);

	float damping = (float)lua_tonumber(L, 2);

	car->_springDamping = damping;

	return 1;
}

int lua_addForceToObj(lua_State* L)
{
	Object* obj = (Object*)lua_touserdata(L, 1);

	Playground* pg = (Playground*)lua_touserdata(L, 2);

	float fx = (float)lua_tonumber(L, 3);
	float fy = (float)lua_tonumber(L, 4);
	float fz = (float)lua_tonumber(L, 5);

	b3Body_ApplyForceToCenter(pg->_bodies[obj->_physId], b3Vec3{fx,fy,fz}, true);

	return 1;
}

int lua_isKeyPressed(lua_State* L)
{
	EventHandler* eh = (EventHandler*)lua_touserdata(L, 1);
	int bitshift = (int)lua_tonumber(L, 2);

	if (eh->_pressedKeys & (1 << bitshift)) {
		lua_pushboolean(L, 1);
	}
	else {
		lua_pushboolean(L, 0);
	}
		

	return 1;
}


void Lua::init(lua_State* L, Playground* pg, EventHandler* eh)
{
	luaL_openlibs(L);


	lua_pushlightuserdata(L, pg);
	lua_setglobal(L, "pg");

	lua_pushlightuserdata(L, eh);
	lua_setglobal(L, "eventhandler");

	lua_register(L, "addTexture", lua_addTexture);
	lua_register(L, "addModel", lua_addModel);
	lua_register(L, "addObject", lua_addObject);
	lua_register(L, "convertToCar", lua_convertToCar);
	lua_register(L, "getObjectPointer", lua_getObjectPointer);
	lua_register(L, "setDamping", lua_setDamping);
	lua_register(L, "addForceToObj", lua_addForceToObj);
	lua_register(L, "isKeyPressed", lua_isKeyPressed);
	


	int r = luaL_dofile(L, "D:/Github/raylib_box3d/template/res/textures.lua");

	if (r != LUA_OK) {
		TraceLog(LOG_ERROR, "Lua failed: %s", lua_tostring(L, -1));
		lua_close(L);
	}
}