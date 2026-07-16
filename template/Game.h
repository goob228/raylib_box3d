#ifndef GAME_H
#define GAME_H

#include "WindowHandler.h"
#include "EventHandler.h"
#include "Playground.h"
#include "LuaBind.h"

#ifndef SET_FPS 
#define SET_FPS 60
#endif



typedef struct Game
{


	

	WindowHandler* _windowhandler;
	EventHandler* _eventhandler;
	Playground* _playground;
	lua_State* L;

	bool _running;

	int _targetFPS;
	float _deltaTime;


} Game;


void game_startLoop(struct Game* self);



#endif