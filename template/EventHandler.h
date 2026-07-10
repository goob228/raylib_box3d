#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <cstdint>

#define EH_K_W       (1 << 0)
#define EH_K_S       (1 << 1)
#define EH_K_D       (1 << 2)
#define EH_K_A       (1 << 3)
#define EH_K_SPACE   (1 << 4)
#define EH_K_ESC     (1 << 5)
#define EH_K_QUIT    (1 << 6)	
#define EH_K_RESTART (1 << 7)


class EventHandler
{
public:

	void processInput();

	uint16_t _keys = 0;
	uint16_t _pressedKeys = 0;
	float _mx = 0.0f;;
	float _my = 0.0f;
};


#endif