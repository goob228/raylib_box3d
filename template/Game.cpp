#include "Game.h"


#include "WindowHandler.h"
#include "EventHandler.h"
#include "Playground.h"
#include "LuaBind.h"


Game::Game()
{
	_running = false;
	_windowhandler = nullptr;
	_eventhandler = nullptr;
	_playground = nullptr;
	L = nullptr;

}

void Game::quit()
{

	_playground->cleanUp(_playground);
	_windowhandler->close(_windowhandler);

	lua_close(L);


	_running = false;
	delete _eventhandler;
	delete _windowhandler;
	delete _playground;
}

int Game::init()
{

	_eventhandler = new EventHandler{ 0 };
	if (!_eventhandler) return 1;
	_eventhandler->processInput = (&eh_processInput);

	_windowhandler = new WindowHandler;

	if (!_windowhandler) return 1;
	_windowhandler->init = (&wh_init);
	_windowhandler->startFrame = (&wh_startFrame);
	_windowhandler->endFrame = (&wh_endFrame);
	_windowhandler->close = (&wh_close);

	_windowhandler->screenWidth = 800;
	_windowhandler->screenHeight = 600;

	_playground = new Playground;

	if (!_playground) return 1;

	_playground->init = (&pg_init);
	_playground->addModel = (&pg_addModel);
	_playground->addTexture = (&pg_addTexture);
	_playground->addObject = (&pg_addObject);
	_playground->addObjectPointer = (&pg_addObjectPointer);
	_playground->render = (&pg_render);
	_playground->update = (&pg_update);
	_playground->cleanUp = (&pg_cleanUp);

	_running = true;

	_windowhandler->init(_windowhandler, _targetFPS);
	_playground->init(_playground, _targetFPS);

	L = luaL_newstate();

	Lua::init(L, _playground, _eventhandler);

	return 0;

}


void Game::startLoop()
{

	if (init()) return;

	

	

	while (_running) {

		_eventhandler->processInput(_eventhandler);

		if (_eventhandler->_keys & EH_K_QUIT) {
			_running = false;
		}

		if (_eventhandler->_keys & EH_K_RESTART) {
			quit();
			if (init()) return;
			continue;
		}

		lua_getglobal(L, "update");
		lua_pcall(L, 0, 0, 0);

		_playground->update(_playground, _eventhandler);
		
		_windowhandler->startFrame(_windowhandler);
		_playground->render(_playground, _windowhandler);
		_windowhandler->endFrame(_windowhandler);

	}

	
	
	quit();


	return;
}

