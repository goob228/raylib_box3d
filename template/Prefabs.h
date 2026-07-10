#ifndef PREFABS_H
#define PREFABS_h

#include "Object.h"

#include <box3d/box3d.h>
#include <lua/lua.hpp>

#include "Animation.h"

#define CAR_WHEEL_COUNT 8

class Car;

class Wheel : public Object
{
public:

	static Wheel* create(Object* object);

	void update(Playground* playground) override;

	b3Vec3 _defaultPos = { 0 };

	Car* _car = nullptr;

	
	float _springLen = 0.0f;
	float _prevHeight = 0.0f;
	float _angle = 0.0f;

	float _weight = 60.0f;
};


class Car : public Object
{ 
public:

	static Car* create(Object* object, Playground* playground);

	void update(Playground* playground) override;

	void steer(float angleDeg);
	


	float _springLen = 1.0f;

	float _springStiffness = 800.0f;

	float _springDamping = 0.9f;

	float _tireFriction = 0.8f;

	

	bool _accelerating = false;
	bool _braking = false;
	LookUpCurve _torqueCurve = { 0 };
	float _torque = 3000.0f;
	float _maxSpeed = 300.0f;

	Wheel* _wheels[CAR_WHEEL_COUNT] = { 0 };

	int _wheelCount = 0;

};




#endif