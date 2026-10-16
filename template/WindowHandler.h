#ifndef WINDOWHANDLER_H
#define WINDOWHANDLER_H






typedef struct WindowHandler {
	void (*init)(struct WindowHandler* self, int FPS);
	void (*startFrame)(struct WindowHandler* self);
	void (*endFrame)(struct WindowHandler* self);
	void (*close)(struct WindowHandler* self);

	int screenWidth;
	int screenHeight;
} WindowHandler;


void wh_init(struct WindowHandler* self, int FPS);

void wh_enableCursor();
void wh_disableCursor();

#endif