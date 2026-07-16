#include "Game.h"


#include "WindowHandler.h"
#include "EventHandler.h"
#include "Playground.h"
#include "LuaBind.h"

#include <malloc.h>



void game_quit(struct Game* self)
{

	self->_playground->cleanUp(self->_playground);
	self->_windowhandler->close(self->_windowhandler);

	lua_close(self->L);


	self->_running = false;
	free(self->_eventhandler);
	free(self->_windowhandler);
	free(self->_playground);
}

int game_init(struct Game* self)
{
	self->_running = false;
	self->_windowhandler = (WindowHandler*)0;
	self->_eventhandler = (EventHandler*)0;
	self->_playground = (Playground*)0;
	self->L = (lua_State*)0;

	self->_running = false;

	self->_targetFPS = SET_FPS;
	self->_deltaTime = 1.0f / (float)self->_targetFPS;

	self->_eventhandler = (EventHandler*)malloc(sizeof(EventHandler));
	if (!self->_eventhandler) return 1;
	self->_eventhandler->processInput = (&eh_processInput);

	self->_windowhandler = (WindowHandler*)malloc(sizeof(WindowHandler));

	if (!self->_windowhandler) return 1;
	self->_windowhandler->init = (&wh_init);
	self->_windowhandler->startFrame = (&wh_startFrame);
	self->_windowhandler->endFrame = (&wh_endFrame);
	self->_windowhandler->close = (&wh_close);

	self->_windowhandler->screenWidth = 800;
	self->_windowhandler->screenHeight = 600;

	self->_playground = (Playground*)malloc(sizeof(Playground));

	if (!self->_playground) return 1;

	self->_playground->init = (&pg_init);
	self->_playground->addModel = (&pg_addModel);
	self->_playground->addTexture = (&pg_addTexture);
	self->_playground->addObject = (&pg_addObject);
	self->_playground->addObjectPointer = (&pg_addObjectPointer);
	self->_playground->render = (&pg_render);
	self->_playground->update = (&pg_update);
	self->_playground->cleanUp = (&pg_cleanUp);

	self->_running = true;

	self->_windowhandler->init(self->_windowhandler, self->_targetFPS);
	self->_playground->init(self->_playground, self->_targetFPS);

	self->L = luaL_newstate();

	lual_init(self->L, self->_playground, self->_eventhandler);

	return 0;

}


void game_startLoop(struct Game* self)
{

	if (game_init(self)) return;

	

	

	while (self->_running) {

		self->_eventhandler->processInput(self->_eventhandler);

		if (self->_eventhandler->_keys & EH_K_QUIT) {
			self->_running = false;
		}

		if (self->_eventhandler->_keys & EH_K_RESTART) {
			game_quit(self);
			if (game_init(self)) return;
			continue;
		}

		lua_getglobal(self->L, "update");
		lua_pcall(self->L, 0, 0, 0);

		self->_playground->update(self->_playground, self->_eventhandler);
		
		self->_windowhandler->startFrame(self->_windowhandler);
		self->_playground->render(self->_playground, self->_windowhandler);
		self->_windowhandler->endFrame(self->_windowhandler);

	}

	
	
	game_quit(self);


	return;
}

