#include "Game.h"


#ifdef NDEBUG
int WinMain()
#else 
int main()
#endif
{
	Game game;

	game_startLoop(&game);


	return 0;
}