#ifndef CAMERA_H
#define CAMERA_H

#include <raylib.h>
#include <rcamera.h>

#include <stdbool.h>
#include "Object.h"


typedef enum {
	CAM_THIRD_PERSON,
	CAM_FIRST_PERSON,
	CAM_TYPE_COUNT
} Cam_type;


typedef struct
{
	void (*init)           (Object* self);
	void (*startFrame)     (Object* self);
	void (*endFrame)       (Object* self);
	b3Vec3 (*getForward)   (Object* self);
	b3Vec3 (*getRight)     (Object* self);

	Vector3 (*rotatedPos)  (Object* self, Vector3 pos);

	Camera _cam;
	int _camMode;

	Vector3 _target;
	Vector3 _up;

	float _pitch;
	float _yaw;
	float _sensitivity;

	float dist;

	Cam_type type;

} CameraData;

void gc_init(Object* self);


#endif