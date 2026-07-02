#include "EventHandler.h"

#include <raylib.h>



void EventHandler::processInput()
{
	_keys = 0;

	if (IsKeyPressed(KEY_W)) _keys |= EH_K_W;
	if (IsKeyPressed(KEY_S)) _keys |= EH_K_S;
	if (IsKeyPressed(KEY_D)) _keys |= EH_K_D;
	if (IsKeyPressed(KEY_A)) _keys |= EH_K_A;
	if (WindowShouldClose()) _keys |= EH_K_QUIT;

}