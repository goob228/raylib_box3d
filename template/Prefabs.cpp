#include "Prefabs.h"

#include <cmath>


#include "Object.h"

#include <box3d/box3d.h>
#include <lua/lua.hpp>

#include "Playground.h"
#include "Animation.h"

#include <iostream>


Vector3 b3Vec3TOVector3(b3Vec3 v)
{
	return Vector3{ v.x, v.y, v.z };
}

b3Vec3 Vector3TOb3Vec3(Vector3 v)
{
	return b3Vec3{ v.x, v.y, v.z };
}

Wheel* Wheel::create(Object* object)
{
	Wheel* wheel = new Wheel();
	wheel->_transform = object->_transform;
	wheel->_pos = object->_pos;
	wheel->_defaultPos = b3Vec3{wheel->_pos.x, wheel->_pos.y, wheel->_pos.z};
	wheel->_rot = object->_rot;
	wheel->_scale = object->_scale;
	wheel->_alive = object->_alive;
	wheel->_type = object->_type;
	wheel->_physId = object->_physId;
	wheel->_texId = object->_texId;
	wheel->_modelId = object->_modelId;

	delete object;

	return wheel;
}

void Wheel::update(Playground* playground)
{
	if (_car) {
		_pos = Vector3{ _defaultPos.x, _defaultPos.y - _prevHeight, _defaultPos.z};
		_rot = QuaternionFromEuler(0.0f, _angle, 0.0f);
	}
	updateMatrix();
}


void vecToWheel(b3Vec3* vec, float angel)
{
	vec->x = vec->x * cosf(angel) - vec->z * sinf(angel);

	vec->z = vec->z * cosf(angel) + vec->z * sinf(angel);
}


Car* Car::create(Object* object, Playground* playground)
{
	Car* car = new Car();
	car->_transform = object->_transform;
	car->_pos = object->_pos;
	car->_rot = object->_rot;
	car->_scale = object->_scale;
	car->_alive = object->_alive;
	car->_type = object->_type;
	car->_physId = object->_physId;
	car->_texId = object->_texId;
	car->_modelId = object->_modelId;

	car->_springStiffness = 10.0f * b3Body_GetMass(playground->_bodies[car->_physId]) * 0.25f * 4.0f;

	std::cout << "MASS: " << b3Body_GetMass(playground->_bodies[car->_physId]) << std::endl;

	b3Vec3 masscen = b3Body_GetLocalCenterOfMass(playground->_bodies[car->_physId]);
	std::cout << "mx: " << masscen.x << "  my: " << masscen.y << "  mz: " << masscen.z << std::endl;
 
	car->_springDamping = 5.0f;


	car->_torqueCurve.len = 11;
	float mv[] = { 0.5f, 0.6f, 0.8f, 0.95f, 1.0f ,1.0f ,1.0f, 0.9f, 0.5f, 0.2f, 0.0f };
	for (int i = 0; i < car->_torqueCurve.len; i++) {
		car->_torqueCurve.val[i] = mv[i];
	}

	delete object;

	return car;
}


void Car::steer(float angleDeg)
{
	_wheels[2]->_angle = angleDeg * DEG2RAD;
	_wheels[3]->_angle = angleDeg * DEG2RAD;
}



void Car::update(Playground* playground)
{
	Object::update(playground);

	b3BodyId bid = playground->_bodies[_physId];

	float bodyMass = b3Body_GetMass(bid);

	b3WorldTransform transform = b3Body_GetTransform(bid);

	b3Pos pos = b3Body_GetPosition(bid);

	b3Vec3 linVel = b3Body_GetLinearVelocity(bid);

	b3Vec3 anVel =  b3Body_GetAngularVelocity(bid);

	b3Vec3 wheelVel = { 0 };


	for (int i = 0; i < _wheelCount; i++) {
		if (!_wheels[i]) continue;
		wheelVel = b3Body_GetLocalPointVelocity(bid, _wheels[i]->_defaultPos);
		wheelVel = b3Body_GetLocalVector(bid, wheelVel);
		vecToWheel(&wheelVel, _wheels[i]->_angle);
		b3Pos rayorigin = b3Body_GetWorldPoint(bid, _wheels[i]->_defaultPos);
		b3Vec3 raytranslation = -_wheels[i]->_springLen * b3Vec3_axisY;

		raytranslation = b3Body_GetWorldVector(bid, raytranslation);
		

		b3QueryFilter skipTeamFilter = { 1, ~2u };
		b3RayResult result = b3World_CastRayClosest(playground->_worldId, rayorigin, raytranslation, skipTeamFilter);

		b3Vec3 force = b3Vec3{ 0 };
		b3Vec3 springVel = b3Vec3{ 0 };

		b3Vec3 springforce = b3Vec3{ 0 };

		b3Vec3 frictionforce = b3Vec3{ 0 };

		b3Vec3 torqueforce = b3Vec3{ 0 };


		if (result.hit == false) {
			playground->_lines[i * 2 + 0].x = rayorigin.x;
			playground->_lines[i * 2 + 0].y = rayorigin.y;
			playground->_lines[i * 2 + 0].z = rayorigin.z;

			playground->_lines[i * 2 + 1].x = rayorigin.x + raytranslation.x;
			playground->_lines[i * 2 + 1].y = rayorigin.y + raytranslation.y;
			playground->_lines[i * 2 + 1].z = rayorigin.z + raytranslation.z;

			_wheels[i]->_prevHeight = _wheels[i]->_springLen;

		}
		else {
			playground->_lines[i * 2 + 0].x = rayorigin.x;
			playground->_lines[i * 2 + 0].y = rayorigin.y;
			playground->_lines[i * 2 + 0].z = rayorigin.z;

			playground->_lines[i * 2 + 1].x = result.point.x;
			playground->_lines[i * 2 + 1].y = result.point.y;
			playground->_lines[i * 2 + 1].z = result.point.z;

			springVel = wheelVel.y * b3Vec3_axisY;
			springVel = b3Body_GetWorldVector(bid, springVel);
			springforce = (result.fraction - 1.0f) * raytranslation * _springStiffness - _springDamping * springVel;
			springforce = b3Dot(springforce, result.normal) * result.normal;


			frictionforce = -_tireFriction * wheelVel.x * playground->_targetFPS * b3Vec3_axisX * _wheels[i]->_weight;
			frictionforce = b3Body_GetWorldVector(bid, frictionforce);

			if (_accelerating) {
				torqueforce = _torqueCurve.evaluate(wheelVel.z / _maxSpeed) * _torque * b3Vec3_axisZ;
				vecToWheel(&torqueforce, -_wheels[i]->_angle);
				torqueforce = b3Body_GetWorldVector(bid, torqueforce);
			}
			

			force += springforce;

			force += frictionforce;

			force += torqueforce;


			b3Body_ApplyForce(bid, force, result.point, true);


			_wheels[i]->_prevHeight = _wheels[i]->_springLen * result.fraction;
		}
		float factor1 = 10.0f;


		playground->_lines[8 + i * 2 + 0].x = rayorigin.x;
		playground->_lines[8 + i * 2 + 0].y = rayorigin.y;
		playground->_lines[8 + i * 2 + 0].z = rayorigin.z;
		
		playground->_lines[8 + i * 2 + 1].x = rayorigin.x + springforce.x * factor1 / _springStiffness;
		playground->_lines[8 + i * 2 + 1].y = rayorigin.y + springforce.y * factor1 / _springStiffness;
		playground->_lines[8 + i * 2 + 1].z = rayorigin.z + springforce.z * factor1 / _springStiffness;

		float factor2 = 0.1f;
		
		playground->_lines[16 + i * 2 + 0].x = rayorigin.x;
		playground->_lines[16 + i * 2 + 0].y = rayorigin.y;
		playground->_lines[16 + i * 2 + 0].z = rayorigin.z;

		playground->_lines[16 + i * 2 + 1].x = rayorigin.x + frictionforce.x * factor2 / _wheels[i]->_weight;
		playground->_lines[16 + i * 2 + 1].y = rayorigin.y + frictionforce.y * factor2 / _wheels[i]->_weight;// -weight * 0.25;
		playground->_lines[16 + i * 2 + 1].z = rayorigin.z + frictionforce.z * factor2 / _wheels[i]->_weight;
		
	}
	
	_accelerating = false;
	_braking = false;
}