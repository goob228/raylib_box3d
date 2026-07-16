#ifndef CAMERA_H
#define CAMERA_H

#include <raylib.h>
#include <rcamera.h>

#include <stdbool.h>
#include "Object.h"




typedef struct GameCamera
{

	OBJECT_FIELDS

	void (*init)(struct GameCamera* self);
	void (*startFrame)(struct GameCamera* self);
	void (*endFrame)(struct GameCamera* self);

	Vector3 (*rotatedPos)(struct GameCamera* self, Vector3 pos);
	
	Camera _cam;
	int _camMode;

	Vector3 _target;
	Vector3 _up;

	float _pitch;
	float _yaw;
	float _sensitivity;

} GameCamera;

void gc_init(struct GameCamera* self);


#endif