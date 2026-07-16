#include "WindowHandler.h"

#include <raylib.h>
#include <rlgl.h>

void wh_init(struct WindowHandler* self, int FPS)
{
	InitWindow(self->screenWidth, self->screenHeight, "template");

	SetTargetFPS(FPS);

	DisableCursor();
}

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

