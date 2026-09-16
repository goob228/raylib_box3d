#include "WindowHandler.h"

#include <stdbool.h>

#include <raylib.h>

#include <rlgl.h>

#include "SysCvar.h"
#include "SysCmd.h"
#include "Menu.h"

static void toggleVsync(Cvar* cvar)
{	
	
	if (cvar->valueb == true)
		SetWindowState(FLAG_VSYNC_HINT);

		
	else if (cvar->valueb == false)
		ClearWindowState(FLAG_VSYNC_HINT);
		
}

static void changeWidthHeight(Cvar* cvar)
{
	SetWindowSize(cv_width.valuei, cv_height.valuei);
}


void wh_startFrame(struct WindowHandler* self)
{
	BeginDrawing();
	ClearBackground(BLACK);
}

void wh_endFrame(struct WindowHandler* self)
{
	DrawFPS(1, 1);
	EndDrawing();
}

void wh_close(struct WindowHandler* self)
{
	CloseWindow();
}

void wh_init(struct WindowHandler* self, int FPS)
{
	

	self->startFrame = (&wh_startFrame);
	self->endFrame = (&wh_endFrame);
	self->close = (&wh_close);	

	self->screenWidth = cv_width.valuei;
	self->screenHeight = cv_height.valuei;

	SetTraceLogCallback(MyTraceLog);

	setCvarCallback(&cv_vsync, toggleVsync);
	setCvarCallback(&cv_width, changeWidthHeight);
	setCvarCallback(&cv_height, changeWidthHeight);
	
	//SetConfigFlags(FLAG_MSAA_4X_HINT); 
	InitWindow(self->screenWidth, self->screenHeight, "template");
	SetTargetFPS(cv_FPS.valuei);

	SetExitKey(KEY_NULL);
	DisableCursor();
}

void wh_enableCursor() {
	EnableCursor();
}
void wh_disableCursor() {
	DisableCursor();
}




