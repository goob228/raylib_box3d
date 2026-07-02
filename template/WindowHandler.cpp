#include "WindowHandler.h"

#include <raylib.h>
#include <rlgl.h>



WindowHandler::~WindowHandler()
{
	close();
}

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
	Vector2 ballPosition = Vector2{ 60.0f, 60.0f };
	DrawCircleV(ballPosition, 50, MAROON);
}

void WindowHandler::endFrame()
{
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