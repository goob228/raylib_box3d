#ifndef WINDOWHANDLER_H
#define WINDOWHANDLER_H


#include <raylib.h>


class WindowHandler
{
public:

	~WindowHandler();

	void init(int FPS);
	void startFrame();
	void endFrame();
	void close();

	void drawBox(float x, float y, float z, float scl);

	void drawSphere(float x, float y, float z, float r);

	int screenWidth = 800;
	int screenHeight = 600;
};


#endif