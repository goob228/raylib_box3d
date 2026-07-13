#include "WindowHandler.h"

#include <raylib.h>
#include <rlgl.h>





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

void WindowHandler::drawBox(float x, float y, float z, float scl)
{
	Vector3 pos = Vector3{ x, y, z };
	DrawCube(pos, scl, scl, scl, RED);
}

void WindowHandler::drawSphere(float x, float y, float z, float r)
{
	Vector3 pos = Vector3{ x, y, z };
	DrawSphere(pos, r, BLUE);
}