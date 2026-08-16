#include "WindowHandler.h"

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <rlgl.h>


void wh_startFrame(struct WindowHandler* self)
{
	BeginDrawing();
	ClearBackground(SKYBLUE);
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

	self->screenWidth = 800;
	self->screenHeight = 600;

	SetConfigFlags(FLAG_VSYNC_HINT); 
	InitWindow(self->screenWidth, self->screenHeight, "template");


	DisableCursor();
}


