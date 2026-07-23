#ifndef CAMERA_H
#define CAMERA_H

#include <raylib.h>
#include <rcamera.h>

#include <stdbool.h>
#include "Object.h"





typedef struct
{
	void (*init)(Object* self);
	void (*startFrame)(Object* self);
	void (*endFrame)(Object* self);

	Vector3(*rotatedPos)(Object* self, Vector3 pos);

	Camera _cam;
	int _camMode;

	Vector3 _target;
	Vector3 _up;

	float _pitch;
	float _yaw;
	float _sensitivity;
} CameraData;

void gc_init(Object* self);


#endif