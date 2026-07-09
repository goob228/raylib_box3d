#include "EventHandler.h"

#include <raylib.h>



void EventHandler::processInput()
{
	_keys = 0;
	_pressedKeys = 0;

	if (IsKeyDown(KEY_W))     _keys |= EH_K_W;
	if (IsKeyDown(KEY_S))     _keys |= EH_K_S;
	if (IsKeyDown(KEY_D))     _keys |= EH_K_D;
	if (IsKeyDown(KEY_A))     _keys |= EH_K_A;
	if (IsKeyDown(KEY_SPACE)) _keys |= EH_K_SPACE;
	if (IsKeyDown(KEY_R))     _keys |= EH_K_RESTART;
	if (WindowShouldClose())  _keys |= EH_K_QUIT;


	if (IsKeyPressed(KEY_W))     _pressedKeys |= EH_K_W;
	if (IsKeyPressed(KEY_S))     _pressedKeys |= EH_K_S;
	if (IsKeyPressed(KEY_D))     _pressedKeys |= EH_K_D;
	if (IsKeyPressed(KEY_A))     _pressedKeys |= EH_K_A;
	if (IsKeyPressed(KEY_SPACE)) _pressedKeys |= EH_K_SPACE;
	if (IsKeyPressed(KEY_R))     _pressedKeys |= EH_K_RESTART;
	if (WindowShouldClose())     _pressedKeys |= EH_K_QUIT;

}