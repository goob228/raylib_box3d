#ifndef CAMERA_H
#define CAMERA_H

#include <raylib.h>
#include <rcamera.h>


class GameCamera
{
public:
	

	void init();
	void update();
	void startFrame();
	void endFrame();

private:
	
	Camera _cam = Camera{ 0 };
	int _camMode;

};


#endif