#include "WindowHandler.h"

#include <raylib.h>
#include <rlgl.h>



/*
void WindowHandler::init(int FPS)
{
	InitWindow(screenWidth, screenHeight, "template");

	SetTargetFPS(FPS);

	DisableCursor();
}

void WindowHandler::startFrame()
{
	BeginDrawing();
	ClearBackground(SKYBLUE);
}

void WindowHandler::endFrame()
{
	DrawFPS(1, 1);
	EndDrawing();
}

void WindowHandler::close()
{
	CloseWindow();
}
*/

void wh_init(WindowHandler* self, int FPS)
{
	InitWindow(self->screenWidth, self->screenHeight, "template");

	SetTargetFPS(FPS);

	DisableCursor();
}

void wh_startFrame(WindowHandler* self)
{
	BeginDrawing();
	ClearBackground(SKYBLUE);
}

void wh_endFrame(WindowHandler* self)
{
	DrawFPS(1, 1);
	EndDrawing();
}

void wh_close(WindowHandler* self)
{
	CloseWindow();
}


