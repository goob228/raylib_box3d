#include "Game.h"

#include <stdbool.h>
#include <malloc.h>


#include "WindowHandler.h"
#include "EventHandler.h"
#include "Playground.h"
#include "LuaBind.h"
#include "Menu.h"


static bool inMenu = 0;

void game_quit(struct Game* self)
{

	self->_playground->cleanUp(self->_playground);
	self->_windowhandler->close(self->_windowhandler);

	lua_close(self->L);


	self->running = false;
	free(self->_eventhandler);
	free(self->_windowhandler);
	free(self->_playground);
}

int game_init(struct Game* self)
{
	

	self->running = false;
	self->_windowhandler = (WindowHandler*)0;
	self->_eventhandler = (EventHandler*)0;
	self->_playground = (Playground*)0;
	self->L = (lua_State*)0;

	self->running = false;

	self->targetFPS = SET_FPS;
	self->deltaTime = 1.0f / (float)self->targetFPS;

	self->_eventhandler = (EventHandler*)malloc(sizeof(EventHandler));
	if (!self->_eventhandler) return 1;

	self->_windowhandler = (WindowHandler*)malloc(sizeof(WindowHandler));

	if (!self->_windowhandler) return 1;
	wh_init(self->_windowhandler, self->targetFPS);

	mn_initMenu();
	inMenu = false;

	self->_playground = (Playground*)malloc(sizeof(Playground));

	if (!self->_playground) return 1;

	pg_init(self->_playground, self->targetFPS);


	self->running = true;

	

	self->L = luaL_newstate();

	lual_init(self->L, self->_playground, self->_eventhandler);

	

	return 0;

}


void game_startLoop(struct Game* self)
{

	if (game_init(self)) return;

	

	

	while (self->running) {

		eh_processInput(self->_eventhandler);

		if (self->_eventhandler->keys & EH_K_QUIT) {
			self->running = false;
		}

		if (self->_eventhandler->pressedKeys & EH_K_ESC) {
			inMenu = !inMenu;
			if (inMenu) {
				wh_enableCursor();
				mn_restartMenu();
			} else 		wh_disableCursor();
		}

		if (self->_eventhandler->pressedKeys & EH_K_GRAVE) {
			inMenu = true;
			wh_enableCursor();
			mn_openConsole();
			
		}

		if (self->_eventhandler->keys & EH_K_RESTART && !inMenu) {
			game_quit(self);
			if (game_init(self)) return;
			continue;
		}

		if (inMenu) {
			*(self->_eventhandler) = (EventHandler){0};
		}

		lua_getglobal(self->L, "update");
		lua_pcall(self->L, 0, 0, 0);

		self->_playground->update(self->_playground, self->_eventhandler);
		
		self->_windowhandler->startFrame(self->_windowhandler);
		self->_playground->render(self->_playground, self->_windowhandler);
		if (inMenu) {
			mn_drawMenu();
		}
		
		
		
		
		self->_windowhandler->endFrame(self->_windowhandler);

	}

	
	
	game_quit(self);


	return;
}

