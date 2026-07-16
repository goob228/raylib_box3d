#ifndef LUABIND_H
#define LUABIND_H



#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"

#include "Playground.h"
#include "EventHandler.h"


void lual_init(lua_State* L, Playground* pg, EventHandler* eh);


#endif