#include "Prefabs.h"

#include <math.h>


#include "Object.h"

#include <box3d/box3d.h>
#include <rlgl.h>

#include "Playground.h"
#include "Animation.h"

#include <malloc.h>




Object* wheel_create(Object* object)
{
	Object* wheel = object;

	wheel->update = (&wheel_update);

	WheelData* wheeldata = (WheelData*)wheel->data;

	wheeldata->_defaultPos = (b3Vec3){ wheel->_pos.x, wheel->_pos.y, wheel->_pos.z };
	wheeldata->_springLen = 0.0f;
	wheeldata->_prevHeight = 0.0f;
	wheeldata->_angle = 0.0f;
	wheeldata->_weight = 60.0f;

	wheeldata->_speed = 0.0f;
	wheeldata->_radius = 0.45f;
	wheeldata->_YZangle = 0.0f;


	return wheel;
}

void wheel_update(struct Object* obj, Playground* playground)
{
	Object* self = obj;
	WheelData* wheeldata = (WheelData*)self->data;
	if (wheeldata->_car) {
		self->_pos = (Vector3){ wheeldata->_defaultPos.x, 
				wheeldata->_defaultPos.y - wheeldata->_prevHeight + wheeldata->_radius, wheeldata->_defaultPos.z};
		wheeldata->_YZangle += wheeldata->_speed / wheeldata->_radius * playground->_targetDeltaTime;
		wheeldata->_YZangle = fmodf(wheeldata->_YZangle, 2 * PI);
		self->_rot = QuaternionFromEuler(wheeldata->_YZangle, wheeldata->_angle, 0.0f);
	}
	self->updateMatrix((Object*)self);
}


void vecToWheel(b3Vec3* vec, float angel)
{
	vec->x = vec->x * cosf(angel) - vec->z * sinf(angel);

	vec->z = vec->z * cosf(angel) + vec->x * sinf(angel);
}

float zVecFromWheel(b3Vec3* vec, float angel)
{
	return vec->z * cosf(angel) + vec->x * sinf(angel);
}


Object* car_create(Object* object, Playground* playground)
{
	Object* car = object;

	

	CarData* cardata = (CarData*)car->data;

	cardata->steer = (&car_steer);
	cardata->og_update = object->update;
	car->update = (&car_update);


	cardata->_springLen = 1.1f;
	cardata->_springStiffness = 800.0f;
	cardata->_springDamping = 0.9f;
	cardata->_tireFriction = 0.8f;
	cardata->_accelerating = false;
	cardata->_braking = false;
	cardata->_torqueCurve = curve_create();
	cardata->_torque = 40.0f;
	cardata->_maxSpeed = 70.0f;
	cardata->_wheelCount = 0;

	
	
	
	cardata->_springStiffness = 10.0f * b3Body_GetMass(playground->_bodies[car->_physId]) * 20.0f * 0.25f;


	b3Vec3 masscen = b3Body_GetLocalCenter(playground->_bodies[car->_physId]);
 
	cardata->_springDamping = 5.0f;


	cardata->_torqueCurve.len = 11;
	float mv[] = { 0.5f, 0.6f, 0.8f, 0.95f, 1.0f ,1.0f ,1.0f, 0.9f, 0.5f, 0.2f, 0.0f };
	for (int i = 0; i < cardata->_torqueCurve.len; i++) {
		cardata->_torqueCurve.val[i] = mv[i];
	}


	return car;
}


void car_steer(struct Object* self, float angleDeg)
{
	CarData* cardata = (CarData*)self->data;
	if (cardata->_wheelCount >= 2) {
		((WheelData*)cardata->_wheels[0]->data)->_angle = angleDeg * DEG2RAD;
		((WheelData*)cardata->_wheels[1]->data)->_angle = angleDeg * DEG2RAD;
	}
}



void car_update(struct Object* obj, Playground* playground)
{	
	struct Object* self = (struct Object*)obj;
	CarData* cardata = (CarData*)self->data;
	cardata->og_update(obj, playground);

	b3BodyId bid = playground->_bodies[self->_physId];

	float bodyMass = b3Body_GetMass(bid);

	b3WorldTransform transform = b3Body_GetTransform(bid);

	b3Pos pos = b3Body_GetPosition(bid);

	b3Vec3 linVel = b3Body_GetLinearVelocity(bid);

	b3Vec3 anVel =  b3Body_GetAngularVelocity(bid);

	b3Vec3 wheelVel = { 0 };


	for (int i = 0; i < cardata->_wheelCount; i++) {
		if (!cardata->_wheels[i]) continue;
		wheelVel = b3Body_GetLocalPointVelocity(bid, ((WheelData*)cardata->_wheels[i]->data)->_defaultPos);
		wheelVel = b3Body_GetLocalVector(bid, wheelVel);
		vecToWheel(&wheelVel, ((WheelData*)cardata->_wheels[i]->data)->_angle);
		b3Pos rayorigin = b3Body_GetWorldPoint(bid, ((WheelData*)cardata->_wheels[i]->data)->_defaultPos);
		b3Vec3 raytranslation = (b3Vec3){0.0f, -((WheelData*)cardata->_wheels[i]->data)->_springLen, 0.0f };

		raytranslation = b3Body_GetWorldVector(bid, raytranslation);
		

		b3QueryFilter skipTeamFilter = { 1, ~2u };
		b3RayResult result = b3World_CastRayClosest(playground->_worldId, rayorigin, raytranslation, skipTeamFilter);

		b3Vec3 force = (b3Vec3){ 0 };
		b3Vec3 springVel = (b3Vec3){ 0 };

		b3Vec3 springforce = (b3Vec3){ 0 };

		b3Vec3 frictionforce = (b3Vec3){ 0 };

		b3Vec3 torqueforce = (b3Vec3){ 0 };



		if (cardata->_accelerating) {

			((WheelData*)cardata->_wheels[i]->data)->_speed += cardata->_torqueCurve.evaluate(&cardata->_torqueCurve, ((WheelData*)cardata->_wheels[i]->data)->_speed / cardata->_maxSpeed) * cardata->_torque * playground->_targetDeltaTime;

			//torqueforce = _torqueCurve.evaluate(wheelVel.z / _maxSpeed) * _torque * b3Vec3_axisZ;
			//vecToWheel(&torqueforce, -_wheels[i]->_angle);
			//torqueforce = b3Body_GetWorldVector(bid, torqueforce);
		}
		else {
			float basicFriction = 10.0f * playground->_targetDeltaTime;
			if (((WheelData*)cardata->_wheels[i]->data)->_speed > 0.0f) 
				((WheelData*)cardata->_wheels[i]->data)->_speed -= Clamp(basicFriction, 0.0f, ((WheelData*)cardata->_wheels[i]->data)->_speed);
			else 
				((WheelData*)cardata->_wheels[i]->data)->_speed += Clamp(basicFriction, 0.0f, -((WheelData*)cardata->_wheels[i]->data)->_speed);
		}
		

		float maxVelFric = 30.0f * playground->_targetDeltaTime;
		
		

		if (cardata->_braking && i >= 2) {
			((WheelData*)cardata->_wheels[i]->data)->_speed = 0.0f;
			//if (_wheels[i]->_speed > 0.0f) _wheels[i]->_speed -= Clamp(maxVelFric, 0.0f, _wheels[i]->_speed);
			//else _wheels[i]->_speed += Clamp(maxVelFric, 0.0f, -_wheels[i]->_speed);
		}


		if (result.hit == false) {
			playground->_lines[i * 2 + 0].x = rayorigin.x;
			playground->_lines[i * 2 + 0].y = rayorigin.y;
			playground->_lines[i * 2 + 0].z = rayorigin.z;

			playground->_lines[i * 2 + 1].x = rayorigin.x + raytranslation.x;
			playground->_lines[i * 2 + 1].y = rayorigin.y + raytranslation.y;
			playground->_lines[i * 2 + 1].z = rayorigin.z + raytranslation.z;

			((WheelData*)cardata->_wheels[i]->data)->_prevHeight = ((WheelData*)cardata->_wheels[i]->data)->_springLen;

		}
		else {
			playground->_lines[i * 2 + 0].x = rayorigin.x;
			playground->_lines[i * 2 + 0].y = rayorigin.y;
			playground->_lines[i * 2 + 0].z = rayorigin.z;

			playground->_lines[i * 2 + 1].x = result.point.x;
			playground->_lines[i * 2 + 1].y = result.point.y;
			playground->_lines[i * 2 + 1].z = result.point.z;

			springVel = (b3Vec3){ 0.0f, wheelVel.y, 0.0f };
			springVel = b3Body_GetWorldVector(bid, springVel);
			float spring_factor_pid = ((result.fraction - 1.0f) * cardata->_springStiffness);
			springforce = b3MulSub(b3Mul(raytranslation, (b3Vec3){ spring_factor_pid , spring_factor_pid
			, spring_factor_pid
			}), cardata->_springDamping, springVel);
			float scalarSpringForce = b3Dot(springforce, result.normal);
			springforce = b3Mul(result.normal, (b3Vec3) { scalarSpringForce, scalarSpringForce
			, scalarSpringForce
			}); // scalarSpringForce * result.normal;

			float diff = ((WheelData*)cardata->_wheels[i]->data)->_speed - wheelVel.z;

			float pifagor_2 = wheelVel.x * wheelVel.x + diff * diff;
			float pifagor = sqrtf(pifagor_2);
			if (pifagor_2 >= maxVelFric * maxVelFric) {
				((WheelData*)cardata->_wheels[i]->data)->_sliding = true;
				float factor = maxVelFric / pifagor;
				wheelVel.x *= factor;
				diff *= factor;
			}
			else {
				((WheelData*)cardata->_wheels[i]->data)->_sliding = false;
			}

			//if (_accelerating);
			((WheelData*)cardata->_wheels[i]->data)->_speed -= diff;

			frictionforce = (b3Vec3){ -wheelVel.x * (float)playground->_targetFPS * bodyMass * 0.25f, 0.0f, 0.0f }; //-wheelVel.x * (float)playground->_targetFPS * b3Vec3_axisX * bodyMass * 0.25f;
			vecToWheel(&frictionforce, -((WheelData*)cardata->_wheels[i]->data)->_angle);
			frictionforce = b3Body_GetWorldVector(bid, frictionforce);
			
			

			torqueforce = (b3Vec3){ 0.0f, 0.0f, diff * (float)playground->_targetFPS * bodyMass * 0.25f };
				//diff * (float)playground->_targetFPS * b3Vec3_axisZ * bodyMass * 0.25f;
			vecToWheel(&torqueforce, -((WheelData*)cardata->_wheels[i]->data)->_angle);
			torqueforce = b3Body_GetWorldVector(bid, torqueforce);
			
			

			force = (b3Vec3){	springforce.x + frictionforce.x + torqueforce.x,
								springforce.y + frictionforce.y + torqueforce.y,
								springforce.z + frictionforce.z + torqueforce.z };

			b3Body_ApplyForce(bid, force, result.point, true);


			((WheelData*)cardata->_wheels[i]->data)->_prevHeight = ((WheelData*)cardata->_wheels[i]->data)->_springLen * result.fraction;
		}
		float factor1 = 10.0f;


		playground->_lines[8 + i * 2 + 0].x = rayorigin.x;
		playground->_lines[8 + i * 2 + 0].y = rayorigin.y;
		playground->_lines[8 + i * 2 + 0].z = rayorigin.z;
		
		playground->_lines[8 + i * 2 + 1].x = rayorigin.x + springforce.x * factor1 / cardata->_springStiffness;
		playground->_lines[8 + i * 2 + 1].y = rayorigin.y + springforce.y * factor1 / cardata->_springStiffness;
		playground->_lines[8 + i * 2 + 1].z = rayorigin.z + springforce.z * factor1 / cardata->_springStiffness;

		float factor2 = 0.1f;
		
		playground->_lines[16 + i * 2 + 0].x = rayorigin.x;
		playground->_lines[16 + i * 2 + 0].y = rayorigin.y;
		playground->_lines[16 + i * 2 + 0].z = rayorigin.z;

		playground->_lines[16 + i * 2 + 1].x = rayorigin.x + frictionforce.x * factor2 / ((WheelData*)cardata->_wheels[i]->data)->_weight;
		playground->_lines[16 + i * 2 + 1].y = rayorigin.y + frictionforce.y * factor2 / ((WheelData*)cardata->_wheels[i]->data)->_weight;// -weight * 0.25;
		playground->_lines[16 + i * 2 + 1].z = rayorigin.z + frictionforce.z * factor2 / ((WheelData*)cardata->_wheels[i]->data)->_weight;
		
	}
	
	cardata->_accelerating = false;
	cardata->_braking = false;
}


