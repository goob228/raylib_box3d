#include "Camera.h"

#include <raylib.h>
#include <rcamera.h>


void GameCamera::init()
{
	
	_cam.position = Vector3{ 0.0f, 2.0f, 10.0f };

	_cam.target = Vector3{ 0.0f, 2.0f, 0.0f };
	_cam.up = Vector3{ 0.0f, 1.0f, 0.0f };
	_cam.fovy = 90.0f;
	_cam.projection = CAMERA_PERSPECTIVE;
	
	_camMode = CAMERA_THIRD_PERSON;

}

void GameCamera::update()
{
	UpdateCamera(&_cam, _camMode);
}

void GameCamera::startFrame()
{
	BeginMode3D(_cam);
	DrawPlane(Vector3{ 0.0f, 0.0f, 0.0f }, Vector2{ 32.0f, 32.0f }, LIGHTGRAY); // Draw ground
	DrawCube(Vector3 { -16.0f, 2.5f, 0.0f }, 1.0f, 5.0f, 32.0f, BLUE);     // Draw a blue wall
	DrawCube(Vector3 { 16.0f, 2.5f, 0.0f }, 1.0f, 5.0f, 32.0f, LIME);      // Draw a green wall
	DrawCube(Vector3 { 0.0f, 2.5f, 16.0f }, 32.0f, 5.0f, 1.0f, GOLD);
}

void GameCamera::endFrame()
{
	EndMode3D();
}
