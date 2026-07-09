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

	_playground->cleanUp();
	_windowhandler->close();

	lua_close(L);


	_running = false;
	delete _eventhandler;
	delete _windowhandler;
	delete _playground;
}

int Game::init()
{

	_eventhandler = new EventHandler();

	if (!_eventhandler) return 1;


	_windowhandler = new WindowHandler();

	if (!_windowhandler) return 1;

	_playground = new Playground();

	if (!_playground) return 1;

	_running = true;

	_windowhandler->init(_targetFPS);
	_playground->init(_targetFPS);

	L = luaL_newstate();

	Lua::init(L, _playground, _eventhandler);

	return 0;

}


void Game::startLoop()
{

	if (init()) return;

	

	

	while (_running) {

		_eventhandler->processInput();

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

		_playground->update(_eventhandler);
		
		_windowhandler->startFrame();
		_playground->render(_windowhandler);
		_windowhandler->endFrame();

	}

	
	
	quit();


	return;
}

