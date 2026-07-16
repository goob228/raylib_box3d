#ifndef LUABIND_H
#define LUABIND_H

#include <lua/lua.hpp>

#include "Playground.h"
#include "EventHandler.h"


namespace Lua
{
	void init(lua_State* L, Playground* pg, EventHandler* eh);

	

}


#endif