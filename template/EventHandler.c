#include "EventHandler.h"

#include <raylib.h>



void eh_processInput(struct EventHandler * self)
{
	self->_keys = 0;
	self->_pressedKeys = 0;

	if (IsKeyDown(KEY_W))     self->_keys |= EH_K_W;
	if (IsKeyDown(KEY_S))     self->_keys |= EH_K_S;
	if (IsKeyDown(KEY_D))     self->_keys |= EH_K_D;
	if (IsKeyDown(KEY_A))     self->_keys |= EH_K_A;
	if (IsKeyDown(KEY_E))     self->_keys |= EH_K_E;
	if (IsKeyDown(KEY_LEFT_SHIFT))     self->_keys |= EH_K_SHIFT;
	if (IsKeyDown(KEY_SPACE)) self->_keys |= EH_K_SPACE;
	if (IsKeyDown(KEY_R))     self->_keys |= EH_K_RESTART;
	if (WindowShouldClose())  self->_keys |= EH_K_QUIT;


	if (IsKeyPressed(KEY_W))     self->_pressedKeys |= EH_K_W;
	if (IsKeyPressed(KEY_S))     self->_pressedKeys |= EH_K_S;
	if (IsKeyPressed(KEY_D))     self->_pressedKeys |= EH_K_D;
	if (IsKeyPressed(KEY_A))     self->_pressedKeys |= EH_K_A;
	if (IsKeyPressed(KEY_E))     self->_pressedKeys |= EH_K_E;
	if (IsKeyPressed(KEY_LEFT_SHIFT))     self->_pressedKeys |= EH_K_SHIFT;
	if (IsKeyPressed(KEY_SPACE)) self->_pressedKeys |= EH_K_SPACE;
	if (IsKeyPressed(KEY_R))     self->_pressedKeys |= EH_K_RESTART;
	if (WindowShouldClose())     self->_pressedKeys |= EH_K_QUIT;

	Vector2 mdel = GetMouseDelta();

	self->_mx = mdel.x;
	self->_my = mdel.y;
}

