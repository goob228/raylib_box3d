#ifndef WINDOWHANDLER_H
#define WINDOWHANDLER_H


#include <raylib.h>

/*
class WindowHandler
{
public:



	void init(int FPS);
	void startFrame();
	void endFrame();
	void close();

	int screenWidth = 800;
	int screenHeight = 600;
};*/

typedef struct WindowHandler {
	void (*init)(WindowHandler* self, int FPS);
	void (*startFrame)(WindowHandler* self);
	void (*endFrame)(WindowHandler* self);
	void (*close)(WindowHandler* self);

	int screenWidth = 800;
	int screenHeight = 600;
};



void wh_init(WindowHandler* self, int FPS);

void wh_startFrame(WindowHandler* self);

void wh_endFrame(WindowHandler* self);

void wh_close(WindowHandler* self);


#endif