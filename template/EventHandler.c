#include "EventHandler.h"

#include <raylib.h>



void eh_processInput(struct EventHandler * self)
{
	self->keys = 0;
	self->pressedKeys = 0;

	if (IsKeyDown(KEY_W))     self->keys |= EH_K_W;
	if (IsKeyDown(KEY_S))     self->keys |= EH_K_S;
	if (IsKeyDown(KEY_D))     self->keys |= EH_K_D;
	if (IsKeyDown(KEY_A))     self->keys |= EH_K_A;
	if (IsKeyDown(KEY_E))     self->keys |= EH_K_E;
	if (IsKeyDown(KEY_LEFT_SHIFT))     self->keys |= EH_K_SHIFT;
	if (IsKeyDown(KEY_SPACE)) self->keys |= EH_K_SPACE;
	if (IsKeyDown(KEY_R))     self->keys |= EH_K_RESTART;
	if (WindowShouldClose())  self->keys |= EH_K_QUIT;


	if (IsKeyPressed(KEY_W))     self->pressedKeys |= EH_K_W;
	if (IsKeyPressed(KEY_S))     self->pressedKeys |= EH_K_S;
	if (IsKeyPressed(KEY_D))     self->pressedKeys |= EH_K_D;
	if (IsKeyPressed(KEY_A))     self->pressedKeys |= EH_K_A;
	if (IsKeyPressed(KEY_E))     self->pressedKeys |= EH_K_E;
	if (IsKeyPressed(KEY_LEFT_SHIFT))     self->pressedKeys |= EH_K_SHIFT;
	if (IsKeyPressed(KEY_SPACE)) self->pressedKeys |= EH_K_SPACE;
	if (IsKeyPressed(KEY_R))     self->pressedKeys |= EH_K_RESTART;
	if (WindowShouldClose())     self->pressedKeys |= EH_K_QUIT;

	Vector2 mdel = GetMouseDelta();

	self->mx = mdel.x;
	self->my = mdel.y;
}

