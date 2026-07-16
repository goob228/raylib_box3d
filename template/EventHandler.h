#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define EH_K_W       (1 << 0)
#define EH_K_S       (1 << 1)
#define EH_K_D       (1 << 2)
#define EH_K_A       (1 << 3)
#define EH_K_SPACE   (1 << 4)
#define EH_K_ESC     (1 << 5)
#define EH_K_QUIT    (1 << 6)	
#define EH_K_RESTART (1 << 7)
#define EH_K_E (1 << 8)





typedef struct EventHandler {
	void (*processInput)(struct EventHandler* self);
	uint16_t _keys;
	uint16_t _pressedKeys;
	float _mx;
	float _my;
} EventHandler;



void eh_processInput(struct EventHandler* self);

#ifdef __cplusplus
}
#endif

#endif