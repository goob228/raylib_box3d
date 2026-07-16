#include "Prefabs.h"

#include <math.h>


#include "Object.h"

#include <box3d/box3d.h>
#include <rlgl.h>

#include "Playground.h"
#include "Animation.h"

#include <malloc.h>


Vector3 b3Vec3TOVector3(b3Vec3 v)
{
	return (Vector3){ v.x, v.y, v.z };
}

b3Vec3 Vector3TOb3Vec3(Vector3 v)
{
	return (b3Vec3){ v.x, v.y, v.z };
}

Wheel* wheel_create(Object* object)
{
	Wheel* wheel = (Wheel*)malloc(sizeof(Wheel));
	wheel->_transform = object->_transform;
	wheel->_pos = object->_pos;
	wheel->_defaultPos = (b3Vec3){wheel->_pos.x, wheel->_pos.y, wheel->_pos.z};
	wheel->_rot = object->_rot;
	wheel->_scale = object->_scale;
	wheel->_alive = object->_alive;
	wheel->_type = object->_type;
	wheel->_physId = object->_physId;
	wheel->_texId = object->_texId;
	wheel->_modelId = object->_modelId;

	
	wheel->draw = object->draw;
	wheel->updateMatrix = object->updateMatrix;
	wheel->setParent = object->setParent;
	wheel->update = (&wheel_update);

	wheel->_springLen = 0.0f;
	wheel->_prevHeight = 0.0f;
	wheel->_angle = 0.0f;
	wheel->_weight = 60.0f;

	wheel->_speed = 0.0f;
	wheel->_radius = 0.45f;
	wheel->_YZangle = 0.0f;


	free(object);

	return wheel;
}

void wheel_update(struct Object* obj, Playground* playground)
{
	struct Wheel* self = (struct Wheel*)obj;
	if (self->_car) {
		self->_pos = (Vector3){ self->_defaultPos.x, self->_defaultPos.y - self->_prevHeight + self->_radius, self->_defaultPos.z};
		self->_YZangle += self->_speed / self->_radius * playground->_targetDeltaTime;
		self->_YZangle = fmodf(self->_YZangle, 2 * PI);
		self->_rot = QuaternionFromEuler(self->_YZangle, self->_angle, 0.0f);
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


Car* car_create(Object* object, Playground* playground)
{
	Car* car = (Car*)malloc(sizeof(Car));
	car->_transform = object->_transform;
	car->_pos = object->_pos;
	car->_rot = object->_rot;
	car->_scale = object->_scale;
	car->_alive = object->_alive;
	car->_type = object->_type;
	car->_physId = object->_physId;
	car->_texId = object->_texId;
	car->_modelId = object->_modelId;
	car->_parent = object->_parent;
	

	car->og_update = object->update;
	car->draw = object->draw;
	car->setParent = object->setParent;
	car->updateMatrix = object->updateMatrix;
	car->update = (&car_update);
	car->steer = (&car_steer);


	car->_springLen = 1.1f;
	car->_springStiffness = 800.0f;
	car->_springDamping = 0.9f;
	car->_tireFriction = 0.8f;
	car->_accelerating = false;
	car->_braking = false;
	car->_torqueCurve = curve_create();
	car->_torque = 40.0f;
	car->_maxSpeed = 70.0f;
	car->_wheelCount = 0;

	
	
	
	car->_springStiffness = 10.0f * b3Body_GetMass(playground->_bodies[car->_physId]) * 20.0f * 0.25f;


	b3Vec3 masscen = b3Body_GetLocalCenterOfMass(playground->_bodies[car->_physId]);
 
	car->_springDamping = 5.0f;


	car->_torqueCurve.len = 11;
	float mv[] = { 0.5f, 0.6f, 0.8f, 0.95f, 1.0f ,1.0f ,1.0f, 0.9f, 0.5f, 0.2f, 0.0f };
	for (int i = 0; i < car->_torqueCurve.len; i++) {
		car->_torqueCurve.val[i] = mv[i];
	}

	free(object);

	return car;
}


void car_steer(struct Car* self, float angleDeg)
{
	if (self->_wheelCount >= 2) {
		self->_wheels[0]->_angle = angleDeg * DEG2RAD;
		self->_wheels[1]->_angle = angleDeg * DEG2RAD;
	}
}



void car_update(struct Object* obj, Playground* playground)
{	
	struct Car* self = (struct Car*)obj;
	self->og_update(obj, playground);

	b3BodyId bid = playground->_bodies[self->_physId];

	float bodyMass = b3Body_GetMass(bid);

	b3WorldTransform transform = b3Body_GetTransform(bid);

	b3Pos pos = b3Body_GetPosition(bid);

	b3Vec3 linVel = b3Body_GetLinearVelocity(bid);

	b3Vec3 anVel =  b3Body_GetAngularVelocity(bid);

	b3Vec3 wheelVel = { 0 };


	for (int i = 0; i < self->_wheelCount; i++) {
		if (!self->_wheels[i]) continue;
		wheelVel = b3Body_GetLocalPointVelocity(bid, self->_wheels[i]->_defaultPos);
		wheelVel = b3Body_GetLocalVector(bid, wheelVel);
		vecToWheel(&wheelVel, self->_wheels[i]->_angle);
		b3Pos rayorigin = b3Body_GetWorldPoint(bid, self->_wheels[i]->_defaultPos);
		b3Vec3 raytranslation = (b3Vec3){0.0f, -self->_wheels[i]->_springLen, 0.0f };

		raytranslation = b3Body_GetWorldVector(bid, raytranslation);
		

		b3QueryFilter skipTeamFilter = { 1, ~2u };
		b3RayResult result = b3World_CastRayClosest(playground->_worldId, rayorigin, raytranslation, skipTeamFilter);

		b3Vec3 force = (b3Vec3){ 0 };
		b3Vec3 springVel = (b3Vec3){ 0 };

		b3Vec3 springforce = (b3Vec3){ 0 };

		b3Vec3 frictionforce = (b3Vec3){ 0 };

		b3Vec3 torqueforce = (b3Vec3){ 0 };



		if (self->_accelerating) {

			self->_wheels[i]->_speed += self->_torqueCurve.evaluate(&self->_torqueCurve, self->_wheels[i]->_speed / self->_maxSpeed) * self->_torque * playground->_targetDeltaTime;

			//torqueforce = _torqueCurve.evaluate(wheelVel.z / _maxSpeed) * _torque * b3Vec3_axisZ;
			//vecToWheel(&torqueforce, -_wheels[i]->_angle);
			//torqueforce = b3Body_GetWorldVector(bid, torqueforce);
		}
		else {
			float basicFriction = 10.0f * playground->_targetDeltaTime;
			if (self->_wheels[i]->_speed > 0.0f) self->_wheels[i]->_speed -= Clamp(basicFriction, 0.0f, self->_wheels[i]->_speed);
			else self->_wheels[i]->_speed += Clamp(basicFriction, 0.0f, -self->_wheels[i]->_speed);
		}
		

		float maxVelFric = 30.0f * playground->_targetDeltaTime;
		
		

		if (self->_braking && i >= 2) {
			self->_wheels[i]->_speed = 0.0f;
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

			self->_wheels[i]->_prevHeight = self->_wheels[i]->_springLen;

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
			float spring_factor_pid = ((result.fraction - 1.0f) * self->_springStiffness);
			springforce = b3MulSub(b3Mul(raytranslation, (b3Vec3){ spring_factor_pid , spring_factor_pid
			, spring_factor_pid
			}), self->_springDamping, springVel);
			float scalarSpringForce = b3Dot(springforce, result.normal);
			springforce = b3Mul(result.normal, (b3Vec3) { scalarSpringForce, scalarSpringForce
			, scalarSpringForce
			}); // scalarSpringForce * result.normal;

			float diff = self->_wheels[i]->_speed - wheelVel.z;

			float pifagor_2 = wheelVel.x * wheelVel.x + diff * diff;
			float pifagor = sqrtf(pifagor_2);
			if (pifagor_2 >= maxVelFric * maxVelFric) {
				self->_wheels[i]->_sliding = true;
				float factor = maxVelFric / pifagor;
				wheelVel.x *= factor;
				diff *= factor;
			}
			else {
				self->_wheels[i]->_sliding = false;
			}

			//if (_accelerating);
			self->_wheels[i]->_speed -= diff;

			frictionforce = (b3Vec3){ -wheelVel.x * (float)playground->_targetFPS * bodyMass * 0.25f, 0.0f, 0.0f }; //-wheelVel.x * (float)playground->_targetFPS * b3Vec3_axisX * bodyMass * 0.25f;
			vecToWheel(&frictionforce, -self->_wheels[i]->_angle);
			frictionforce = b3Body_GetWorldVector(bid, frictionforce);
			
			

			torqueforce = (b3Vec3){ 0.0f, 0.0f, diff * (float)playground->_targetFPS * bodyMass * 0.25f };
				//diff * (float)playground->_targetFPS * b3Vec3_axisZ * bodyMass * 0.25f;
			vecToWheel(&torqueforce, -self->_wheels[i]->_angle);
			torqueforce = b3Body_GetWorldVector(bid, torqueforce);
			
			

			force = (b3Vec3){	springforce.x + frictionforce.x + torqueforce.x,
								springforce.y + frictionforce.y + torqueforce.y,
								springforce.z + frictionforce.z + torqueforce.z };

			b3Body_ApplyForce(bid, force, result.point, true);


			self->_wheels[i]->_prevHeight = self->_wheels[i]->_springLen * result.fraction;
		}
		float factor1 = 10.0f;


		playground->_lines[8 + i * 2 + 0].x = rayorigin.x;
		playground->_lines[8 + i * 2 + 0].y = rayorigin.y;
		playground->_lines[8 + i * 2 + 0].z = rayorigin.z;
		
		playground->_lines[8 + i * 2 + 1].x = rayorigin.x + springforce.x * factor1 / self->_springStiffness;
		playground->_lines[8 + i * 2 + 1].y = rayorigin.y + springforce.y * factor1 / self->_springStiffness;
		playground->_lines[8 + i * 2 + 1].z = rayorigin.z + springforce.z * factor1 / self->_springStiffness;

		float factor2 = 0.1f;
		
		playground->_lines[16 + i * 2 + 0].x = rayorigin.x;
		playground->_lines[16 + i * 2 + 0].y = rayorigin.y;
		playground->_lines[16 + i * 2 + 0].z = rayorigin.z;

		playground->_lines[16 + i * 2 + 1].x = rayorigin.x + frictionforce.x * factor2 / self->_wheels[i]->_weight;
		playground->_lines[16 + i * 2 + 1].y = rayorigin.y + frictionforce.y * factor2 / self->_wheels[i]->_weight;// -weight * 0.25;
		playground->_lines[16 + i * 2 + 1].z = rayorigin.z + frictionforce.z * factor2 / self->_wheels[i]->_weight;
		
	}
	
	self->_accelerating = false;
	self->_braking = false;
}




void par_update(struct Particle* self, Playground* playground)
{
	if (self->_onRemove) return;

	self->_timeSeconds += playground->_targetDeltaTime;
	self->_pos = Vector3Add(self->_pos, Vector3Multiply(self->_linVel, (Vector3) { playground->_targetDeltaTime }));
	self->_scale = Vector3Add(self->_scale, Vector3Multiply(self->_scaleVel, (Vector3) { playground->_targetDeltaTime }));
	self->_alpha = self->_alpha + self->_alphaVel * playground->_targetDeltaTime;

	if (self->_timeSeconds > self->_lifeTimeSeconds) self->_onRemove = true;
}

void par_draw(struct Particle* self, Playground* playground)
{
	if (self->_onRemove) return;

	rlDisableDepthMask();
	BeginBlendMode(BLEND_ADDITIVE);
	

	DrawBillboardPro(playground->_camera._cam, playground->_textures[self->_texId],
		(Rectangle){0.0f, 0.0f, (float)playground->_textures[self->_texId].width, (float)playground->_textures[self->_texId].height},
		self->_pos, playground->_camera._cam.up,
		(Vector2){ self->_scale.x, self->_scale.z }, (Vector2){ self->_scale.x*0.5f, self->_scale.z*0.5f }, self->_anVel* self->_timeSeconds, (Color){255, 255, 255, (unsigned char)b3ClampInt((int)self->_alpha, 0, 255)});
	EndBlendMode();
	rlEnableDepthMask();
}


