#include "Prefabs.h"

#include <cmath>


#include "Object.h"

#include <box3d/box3d.h>
#include <lua/lua.hpp>

#include "Playground.h"





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

	car->_springDamping = 5.0f;



	car->_wheelPoses[0].x = -1.0f;
	car->_wheelPoses[0].z = -2.0f;

	car->_wheelPoses[1].x = 1.0f;
	car->_wheelPoses[1].z = -2.0f;

	car->_wheelPoses[2].x = -1.0f;
	car->_wheelPoses[2].z = 2.0f;

	car->_wheelPoses[3].x = 1.0f;
	car->_wheelPoses[3].z = 2.0f;

	car->_wheelPoses[0].y = 0.1f;
	car->_wheelPoses[1].y = 0.1f;
	car->_wheelPoses[2].y = 0.1f;
	car->_wheelPoses[3].y = 0.1f;

	delete object;

	return car;
}




void Car::update(Playground* playground)
{
	Object::update(playground);

	b3BodyId bid = playground->_bodies[_physId];

	b3WorldTransform transform = b3Body_GetTransform(bid);

	b3Pos pos = b3Body_GetPosition(bid);

	b3Vec3 linVel = b3Body_GetLinearVelocity(bid);

	b3Vec3 anVel =  b3Body_GetAngularVelocity(bid);

	b3Vec3 wheelVel;


	for (int i = 0; i < CAR_WHEEL_COUNT; i++) {
		wheelVel = b3Body_GetLocalPointVelocity(bid, _wheelPoses[i]);
		wheelVel = b3Body_GetLocalVector(bid, wheelVel);
		vecToWheel(&wheelVel, _wheelAngel[i]);
		b3Pos rayorigin = b3Body_GetWorldPoint(bid, _wheelPoses[i]);
		b3Vec3 raytranslation = -_wheelHeight * b3Vec3_axisY;

		raytranslation = b3Body_GetWorldVector(bid, raytranslation);
		

		b3QueryFilter skipTeamFilter = { 1, ~2u };
		b3RayResult result = b3World_CastRayClosest(playground->_worldId, rayorigin, raytranslation, skipTeamFilter);

		b3Vec3 force = b3Vec3{ 0 };
		b3Vec3 springVel = b3Vec3{ 0 };

		b3Vec3 springforce = b3Vec3{ 0 };

		b3Vec3 frictionforce = b3Vec3{ 0 };


		if (result.hit == false) {
			playground->_lines[i * 2 + 0].x = rayorigin.x;
			playground->_lines[i * 2 + 0].y = rayorigin.y;
			playground->_lines[i * 2 + 0].z = rayorigin.z;

			playground->_lines[i * 2 + 1].x = rayorigin.x + raytranslation.x;
			playground->_lines[i * 2 + 1].y = rayorigin.y + raytranslation.y;
			playground->_lines[i * 2 + 1].z = rayorigin.z + raytranslation.z;

			_prevHeight[i] = _wheelHeight;

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

			frictionforce = -_tireFriction*wheelVel.x * playground->_targetFPS * b3Vec3_axisX;

			force += b3Dot(springforce, result.normal) * result.normal;

			force += frictionforce;

			b3Body_ApplyForce(bid, force, result.point, true);

			_prevHeight[i] = _wheelHeight * result.fraction;
		}
		float factor1 = 10.0f;


		playground->_lines[8 + i * 2 + 0].x = rayorigin.x;
		playground->_lines[8 + i * 2 + 0].y = rayorigin.y;
		playground->_lines[8 + i * 2 + 0].z = rayorigin.z;
		
		playground->_lines[8 + i * 2 + 1].x = rayorigin.x + wheelVel.x * factor1;
		playground->_lines[8 + i * 2 + 1].y = rayorigin.y + wheelVel.y * factor1;
		playground->_lines[8 + i * 2 + 1].z = rayorigin.z + wheelVel.z * factor1;

		float factor2 = 0.1f;
		/*
		playground->_lines[16 + i * 2 + 0].x = rayorigin.x;
		playground->_lines[16 + i * 2 + 0].y = rayorigin.y;
		playground->_lines[16 + i * 2 + 0].z = rayorigin.z;

		playground->_lines[16 + i * 2 + 1].x = rayorigin.x + force.x * factor2;
		playground->_lines[16 + i * 2 + 1].y = rayorigin.y + force.y * factor2;// -weight * 0.25;
		playground->_lines[16 + i * 2 + 1].z = rayorigin.z + force.z * factor2;
		*/
	}
	
	

}