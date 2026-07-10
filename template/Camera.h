#ifndef CAMERA_H
#define CAMERA_H

#include <raylib.h>
#include <rcamera.h>

#include "Object.h"


class GameCamera : public Object
{
public:
	

	void init();
	void update(Playground* playground) override;
	void draw(Playground* playground) override;
	void startFrame();
	void endFrame();

	Vector3 rotatedPos(Vector3 pos);
	
	Camera _cam = Camera{ 0 };
	int _camMode;

	Vector3 _target = Vector3{ 0.0f, 2.0f, 0.0f };
	Vector3 _up = Vector3{ 0.0f, 1.0f, 0.0f };

	float _pitch = 0.0f;
	float _yaw = 0.0f;
	float _sensitivity = 0.001f;

};


#endif