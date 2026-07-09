#ifndef PREFABS_H
#define PREFABS_h

#include "Object.h"

#include <box3d/box3d.h>
#include <lua/lua.hpp>

#define CAR_WHEEL_COUNT 4


class Car : public Object
{ 
public:

	static Car* create(Object* object, Playground* playground);

	void update(Playground* playground) override;


	


	float _wheelHeight = 0.5f;

	float _springStiffness = 800.0f;

	float _springDamping = 0.9f;

	float _tireFriction = 0.7f;

	b3Vec3 _wheelPoses[CAR_WHEEL_COUNT] = { 0 };

	float _prevHeight[CAR_WHEEL_COUNT] = { _wheelHeight,_wheelHeight,_wheelHeight,_wheelHeight };
	float _wheelAngel[CAR_WHEEL_COUNT] = { 0 };

};

#endif