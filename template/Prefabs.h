#ifndef PREFABS_H
#define PREFABS_h

#include "Object.h"

#include <box3d/box3d.h>

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

	float _speed = 0.0f;
	float _radius = 0.45f;
	float _YZangle = 0.0f;

	bool _sliding = false;
	bool _steering = false;

};


class Car : public Object
{ 
public:

	static Car* create(Object* object, Playground* playground);

	void update(Playground* playground) override;

	void steer(float angleDeg);
	


	float _springLen = 1.1f;

	float _springStiffness = 800.0f;

	float _springDamping = 0.9f;

	float _tireFriction = 0.8f;

	

	bool _accelerating = false;
	bool _braking = false;
	LookUpCurve _torqueCurve = { 0 };
	float _torque = 40.0f;
	float _maxSpeed = 70.0f;

	Wheel* _wheels[CAR_WHEEL_COUNT] = { 0 };

	int _wheelCount = 0;

};


class Particle : public Object
{

public:

	void update(Playground* playground) override;

	void draw(Playground* playground) override;

	Vector3 _linVel = Vector3Zeros;
	Vector3 _scaleVel = Vector3Zeros;
	float _anVel = 0.0f;
	float _timeSeconds = 0.0f;
	float _lifeTimeSeconds = 0.0f;
	float _alpha = 255.0f;
	float _alphaVel = 0.0f;


};



#endif