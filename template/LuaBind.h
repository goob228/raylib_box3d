#ifndef LUABIND_H
#define LUABIND_H

#include <lua/lua.hpp>

#include "Playground.h"
#include "EventHandler.h"


int lua_addTexture(lua_State* L);
int lua_addModel(lua_State* L);
int lua_addObject(lua_State* L);
int lua_convertToCar(lua_State* L);
int lua_getObjectPointer(lua_State* L);

int lua_setDamping(lua_State* L);
int lua_addForceToObj(lua_State* L);

int lua_isKeyPressed(lua_State* L);

namespace Lua
{
	void init(lua_State* L, Playground* pg, EventHandler* eh);

	

}


#endif