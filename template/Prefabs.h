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

#define PLANE_CAPACITY 8

typedef struct
{
	float maxPush;
	bool clipVelocity;
} MoverShapeUserData;

struct PlaneExtra
{
	b3Pos point;
	b3ShapeId shapeId;
};

typedef struct {

	b3CollisionPlane planes[PLANE_CAPACITY];
	struct PlaneExtra planeExtras[PLANE_CAPACITY];
	b3Capsule capsule;
	b3Transform trans;

	b3Vec3 velocity;

	Object* camera;
	Playground* playground;

	float jumpSpeed;
	float maxSpeed;
	float minSpeed;
	float stopSpeed;
	float accelerate;
	float friction;
	float gravity;
	float mass;

	float pogoVelocity;

	int planeCount;
	int totalIterations;
	int ignoreCount;
	b3ShapeId* ignoreShapeIds;

	bool onGround;
	bool sprint;

} CharacterData;


Object* wheel_create(Object* object);

void wheel_update(Object* obj, Playground* playground);

Object* car_create(Object* object, Playground* playground);

void car_steer(Object* self, float angleDeg);

void car_update(Object* self, Playground* playground);

Object* character_create(Object* object, Object* camera, Playground* playground);

#endif