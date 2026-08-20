#ifndef PREFABS_H
#define PREFABS_h

#include "Object.h"

#include <box3d/box3d.h>

#include "Animation.h"


#define CAR_WHEEL_COUNT 6



typedef struct {

	b3Vec3 defaultPos;

	Object* car;


	float springLen;
	float prevHeight;
	float angle;
	float weight;

	float speed;
	float radius;
	float YZangle;

	bool sliding;
	bool steering;

} WheelData;

typedef struct {

	void (*steer)(Object* self, float angleDeg);

	void (*og_update)(Object* self, Playground* playground);

	float springLen;

	float springStiffness;

	float springDamping;

	float tireFriction;



	bool accelerating;
	bool braking;
	LookUpCurve torqueCurve;
	float torque;
	float maxSpeed;

	Object* wheels[CAR_WHEEL_COUNT];

	int wheelCount;
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