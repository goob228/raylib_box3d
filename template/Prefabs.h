#ifndef PREFABS_H
#define PREFABS_h

#include "Object.h"

#include <box3d/box3d.h>

#include "Animation.h"


#define CAR_WHEEL_COUNT 6



typedef struct {

	b3Vec3 _defaultPos;

	Object* _car;


	float _springLen;
	float _prevHeight;
	float _angle;
	float _weight;

	float _speed;
	float _radius;
	float _YZangle;

	bool _sliding;
	bool _steering;

} WheelData;

typedef struct {

	void (*steer)(Object* self, float angleDeg);

	void (*og_update)(Object* self, Playground* playground);

	float _springLen;

	float _springStiffness;

	float _springDamping;

	float _tireFriction;



	bool _accelerating;
	bool _braking;
	LookUpCurve _torqueCurve;
	float _torque;
	float _maxSpeed;

	Object* _wheels[CAR_WHEEL_COUNT];

	int _wheelCount;
} CarData;





Object* wheel_create(Object* object);

void wheel_update(struct Object* obj, Playground* playground);

Object* car_create(Object* object, Playground* playground);

void car_steer(struct Object* self, float angleDeg);

void car_update(struct Object* self, Playground* playground);



#endif