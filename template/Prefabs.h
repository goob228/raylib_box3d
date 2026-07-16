#ifndef PREFABS_H
#define PREFABS_h

#include "Object.h"

#include <box3d/box3d.h>

#include "Animation.h"


#define CAR_WHEEL_COUNT 8

typedef struct Car Car;

typedef struct Wheel
{
	OBJECT_FIELDS

	b3Vec3 _defaultPos;

	Car* _car;

	
	float _springLen;
	float _prevHeight;
	float _angle;
	float _weight;

	float _speed;
	float _radius;
	float _YZangle;

	bool _sliding;
	bool _steering;

} Wheel;


typedef struct Car
{ 
	OBJECT_FIELDS


	void (*steer)(struct Car* self, float angleDeg);
	
	void (*og_update)(struct Object* self, Playground* playground);

	float _springLen;

	float _springStiffness;

	float _springDamping;

	float _tireFriction;

	

	bool _accelerating;
	bool _braking;
	LookUpCurve _torqueCurve;
	float _torque;
	float _maxSpeed;

	Wheel* _wheels[CAR_WHEEL_COUNT];

	int _wheelCount;

} Car;


typedef struct Particle
{
	OBJECT_FIELDS

	Vector3 _linVel;
	Vector3 _scaleVel;
	float _anVel;
	float _timeSeconds;
	float _lifeTimeSeconds;
	float _alpha;
	float _alphaVel;


} Particle;





Wheel* wheel_create(Object* object);

void wheel_update(struct Object* obj, Playground* playground);

Car* car_create(Object* object, Playground* playground);

void car_steer(struct Car* self, float angleDeg);

void car_update(struct Object* self, Playground* playground);

void par_update(struct Particle* self, Playground* playground);

void par_draw(struct Particle* self, Playground* playground);


#endif