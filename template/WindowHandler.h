#ifndef WINDOWHANDLER_H
#define WINDOWHANDLER_H


#include <raylib.h>

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct WindowHandler {
	void (*init)(struct WindowHandler* self, int FPS);
	void (*startFrame)(struct WindowHandler* self);
	void (*endFrame)(struct WindowHandler* self);
	void (*close)(struct WindowHandler* self);

	int screenWidth;
	int screenHeight;
} WindowHandler;



void wh_init(struct WindowHandler* self, int FPS);

void wh_startFrame(struct WindowHandler* self);

void wh_endFrame(struct WindowHandler* self);

void wh_close(struct WindowHandler* self);


#ifdef __cplusplus
}
#endif 

#endif