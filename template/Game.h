#ifndef GAME_H
#define GAME_H

#include "WindowHandler.h"
#include "EventHandler.h"
#include "Playground.h"
#include "LuaBind.h"

#ifndef SET_FPS 
#define SET_FPS 60
#endif


class Game
{
public:

	Game();

	void startLoop();
	void quit();

private:
	
	int init();
	

	WindowHandler* _windowhandler;
	EventHandler* _eventhandler;
	Playground* _playground;
	lua_State* L;

	bool _running = false;

	int _targetFPS = SET_FPS;
	float _deltaTime = 1.0f / (float)_targetFPS;


};


#endif