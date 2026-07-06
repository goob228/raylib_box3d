#ifndef GAME_H
#define GAME_H

#include "WindowHandler.h"
#include "EventHandler.h"
#include "Playground.h"


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

	bool _running = false;

	int _targetFPS = 60;
	float _deltaTime = 1.0f / (float)_targetFPS;


};


#endif