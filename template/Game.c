#include "Game.h"

#include <stdbool.h>
#include <stdint.h>
#include <malloc.h>


#include "WindowHandler.h"
#include "EventHandler.h"
#include "Playground.h"
#include "Menu.h"
#include "Resource.h"
#include "Zone.h"
#include "SysCvar.h"
#include "G_local.h"

Playground* g_playground = NULL;

double alphaBlend = 0.0;

double  host_netinterval = 1.0/60;
int host_netTPS = 60;

static bool inMenu = 0;


static void changeTPS(Cvar* cvar)
{
	host_netinterval = 1.0/cv_TPS.valuei;
	host_netTPS = cv_TPS.valuei;
}

static void changeFPS(Cvar* cvar)
{
	SetTargetFPS(cv_FPS.valuei);
}


void game_quit(struct Game* self)
{
	
	self->_playground->cleanUp(self->_playground);
	self->_windowhandler->close(self->_windowhandler);



	self->running = false;
	free(self->_eventhandler);
	free(self->_windowhandler);
	free(self->_playground);

	g_playground = NULL;

	clearResources();
}

int game_init(struct Game* self)
{
	host_netinterval = 1.0/cv_TPS.valuei;
	host_netTPS = cv_TPS.valuei;

	setCvarCallback(&cv_TPS, changeTPS);
	setCvarCallback(&cv_FPS, changeFPS);


	self->running = false;
	self->_windowhandler = (WindowHandler*)0;
	self->_eventhandler = (EventHandler*)0;
	self->_playground = (Playground*)0;

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

	

	self->_playground = (Playground*)calloc(1, sizeof(Playground));
	

	if (!self->_playground) return 1;

	g_playground = self->_playground;



	

	pg_init(self->_playground, self->targetFPS);
	
	initResources();

	self->running = true;



	return 0;

}

void* raylib_malloc_wrapper(size_t size) {
	return Z_Malloc(size);
}

void* raylib_calloc_wrapper(size_t numofelements, size_t sizeofelement){
	return Z_Malloc(numofelements*sizeofelement);
}

void* raylib_free_wrapper(void* ptr) {
	Z_Free(ptr);
}

void game_startLoop(struct Game* self)
{
	
		
	#define GAME_MEMORY_SIZE (128 * 1024 * 1024)

	void* global_buffer = malloc(GAME_MEMORY_SIZE);

	if (!global_buffer) return;

	Memory_Init(global_buffer, GAME_MEMORY_SIZE);

	initCvars();
    initCmds();

	if (game_init(self)) return;

	


	static double	accumtime = 0.0;
	accumtime = host_netinterval;
	double newTime = GetTime();
	double oldTime = 0.0;
	double time = 0.0;

	while (self->running) {
		oldTime = newTime;
		newTime = GetTime();
		time = newTime - oldTime;
		accumtime += time;
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

		pg_camUpdate(self->_playground, self->_eventhandler);

		while (accumtime >= host_netinterval) {
			pg_update(self->_playground, self->_eventhandler);
			accumtime -= host_netinterval;
			
		}
		
		
		
		alphaBlend = accumtime/host_netinterval;
		alphaBlend = alphaBlend > 0.0 ? (alphaBlend < 1.0 ? alphaBlend : 1.0) : 0.0;
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

