#include "Game.h"


#include "WindowHandler.h"
#include "EventHandler.h"


Game::Game()
{
	_running = false;
	_windowhandler = nullptr;
	_eventhandler = nullptr;
	_playground = nullptr;

}

void Game::quit()
{
	_running = false;
	delete _eventhandler;
	delete _windowhandler;
	delete _playground;
}

int Game::init()
{
	if (_eventhandler) delete _eventhandler;

	_eventhandler = new EventHandler();

	if (!_eventhandler) return 1;


	if (_windowhandler) delete _windowhandler;

	_windowhandler = new WindowHandler();

	if (!_windowhandler) return 1;


	if (_playground) delete _playground;

	_playground = new Playground();

	if (!_playground) return 1;

	return 0;

}


void Game::startLoop()
{
	if (init()) return;

	_running = true;

	_windowhandler->init(_targetFPS);
	_playground->init(_targetFPS);
	

	while (_running) {

		_eventhandler->processInput();

		if (_eventhandler->_keys & EH_K_QUIT) {
			_running = false;
		}


		_playground->update(_eventhandler);
		
		_windowhandler->startFrame();
		_playground->render(_windowhandler);
		_windowhandler->endFrame();

	}

	_playground->cleanUp();
	_windowhandler->close();
	
	quit();



	return;
}

