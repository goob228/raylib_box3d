#include "Game.h"

#include <stdbool.h>
#include <stdint.h>
#include <malloc.h>

#include <flecs.h>

#include "WindowHandler.h"
#include "EventHandler.h"
#include "Menu.h"
#include "Resource.h"
#include "Zone.h"
#include "SysCvar.h"
#include "G_local.h"
#include "MainModule.h"
#include "Camera.h"


double alphaBlend = 0.0;

double  host_netinterval = 1.0/60;
int host_netTPS = 60;

ecs_world_t* g_world;

b3WorldId g_worldid;

EventHandler eventhandlers[5] = {0};


Object camera = {0};

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
	if (g_world) {
		ecs_fini(g_world);
		g_world = NULL;
	}
	
	if (b3World_IsValid(g_worldid)) {
		b3DestroyWorld(g_worldid);
		g_worldid = b3_nullWorldId;
	}

	self->_windowhandler->close(self->_windowhandler);



	self->running = false;
	free(self->_eventhandler);
	free(self->_windowhandler);


	clearResources();
}

int game_init(struct Game* self)
{

	g_world = ecs_init();

	ECS_IMPORT(g_world, MainModule);

	host_netinterval = 1.0/cv_TPS.valuei;
	host_netTPS = cv_TPS.valuei;

	setCvarCallback(&cv_TPS, changeTPS);
	setCvarCallback(&cv_FPS, changeFPS);


	self->running = false;
	self->_windowhandler = (WindowHandler*)0;
	self->_eventhandler = (EventHandler*)0;

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




	b3WorldDef worldDef = b3DefaultWorldDef();
	worldDef.gravity = (b3Vec3){ 0.0f, -10.0f, 0.0f };
	worldDef.enableContinuous = true;

	g_worldid = b3CreateWorld(&worldDef);

	gc_init(&camera);


	
	
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
	
		
	#define GAME_MEMORY_SIZE (32 * 1024 * 1024)

	void* global_buffer = malloc(GAME_MEMORY_SIZE);

	if (!global_buffer) return;

	Memory_Init(global_buffer, GAME_MEMORY_SIZE);

	int startMark = Hunk_LowMark();

	initCvars();
    initCmds();

	if (game_init(self)) return;

	
	ecs_entity_t UpdateMatrixSystem = ecs_system(g_world, {
		.entity = ecs_entity(g_world, {.name = "UpdateMatrixSystem"}),
		.query.terms = {
			{ .id = ecs_id(Drawable) },
			{ .id = ecs_id(Position) },
			{ .id = ecs_id(Rotater) }
		},
		.callback = UpdateMatrix
	});

	ecs_entity_t DrawSystem = ecs_system(g_world, {
		.entity = ecs_entity(g_world, {.name = "DrawSystem"}),
		.query.terms = {
			{ .id = ecs_id(Drawable) },
			{ .id = ecs_id(DrawFunc) }
		},
		.callback = DrawWorld
	});

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
			newTime = GetTime();
			oldTime = newTime;
			accumtime = host_netinterval;
			Hunk_FreeToLowMark(startMark);
			UpdateMatrixSystem = ecs_system(g_world, {
				.entity = ecs_new(g_world),
				.query.terms = {
					{ .id = ecs_id(Drawable) },
					{ .id = ecs_id(Position) },
					{ .id = ecs_id(Rotater) }
				},
				.callback = UpdateMatrix
			});

			DrawSystem = ecs_system(g_world, {
				.entity = ecs_entity(g_world, {.name = "DrawSystem"}),
				.query.terms = {
					{ .id = ecs_id(Drawable) },
					{ .id = ecs_id(DrawFunc) }
				},
				.callback = DrawWorld
			});

			continue;
		}

		if (inMenu) {
			*(self->_eventhandler) = (EventHandler){0};
		}


		ecs_run(g_world, UpdateMatrixSystem, 0, NULL);

		

		ecs_entity_t e = ecs_lookup(g_world, "Player");

		Object* par = ecs_get_mut(g_world, e, Object);

		camera.setParent(&camera, par);
		
		eventhandlers[0] = *(self->_eventhandler);

		camera.update((Object*)&camera);

		while (accumtime >= host_netinterval) {
			ecs_progress(g_world, 0);
			b3World_Step(g_worldid, (float)host_netinterval, 1);
			accumtime -= host_netinterval;
			
		}
		
		
		
		alphaBlend = accumtime/host_netinterval;
		alphaBlend = alphaBlend > 0.0 ? (alphaBlend < 1.0 ? alphaBlend : 1.0) : 0.0;
		self->_windowhandler->startFrame(self->_windowhandler);
		//self->_playground->render(self->_playground, self->_windowhandler);
		ecs_run(g_world, DrawSystem, 0, NULL);
		if (inMenu) {
			mn_drawMenu();
		}
		
		
		
		
		self->_windowhandler->endFrame(self->_windowhandler);

	}

	
	
	game_quit(self);


	return;
}

