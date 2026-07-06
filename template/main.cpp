#include "Game.h"


#ifdef NDEBUG
int WinMain()
#else 
int main()
#endif
{
	Game game;

	game.startLoop();


	return 0;
}