#include "Prefabs.h"

#include <cmath>


#include "Object.h"

#include <box3d/box3d.h>
#include <rlgl.h>

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
	if (this->_car) {
		this->_pos = Vector3{ this->_defaultPos.x, this->_defaultPos.y - this->_prevHeight + this->_radius, this->_defaultPos.z};
		this->_YZangle += this->_speed / this->_radius * playground->_targetDeltaTime;
		this->_YZangle = fmodf(this->_YZangle, 2 * PI);
		this->_rot = QuaternionFromEuler(this->_YZangle, this->_angle, 0.0f);
	}
	updateMatrix();
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

	car->_springStiffness = 10.0f * b3Body_GetMass(playground->_bodies[car->_physId]) * 20.0f * 0.25f;

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
	if (this->_wheelCount >= 2) {
		this->_wheels[0]->_angle = angleDeg * DEG2RAD;
		this->_wheels[1]->_angle = angleDeg * DEG2RAD;
	}
}



void Car::update(Playground* playground)
{
	Object::update(playground);

	b3BodyId bid = playground->_bodies[this->_physId];

	float bodyMass = b3Body_GetMass(bid);

	b3WorldTransform transform = b3Body_GetTransform(bid);

	b3Pos pos = b3Body_GetPosition(bid);

	b3Vec3 linVel = b3Body_GetLinearVelocity(bid);

	b3Vec3 anVel =  b3Body_GetAngularVelocity(bid);

	b3Vec3 wheelVel = { 0 };


	for (int i = 0; i < this->_wheelCount; i++) {
		if (!this->_wheels[i]) continue;
		wheelVel = b3Body_GetLocalPointVelocity(bid, this->_wheels[i]->_defaultPos);
		wheelVel = b3Body_GetLocalVector(bid, wheelVel);
		vecToWheel(&wheelVel, this->_wheels[i]->_angle);
		b3Pos rayorigin = b3Body_GetWorldPoint(bid, this->_wheels[i]->_defaultPos);
		b3Vec3 raytranslation = -this->_wheels[i]->_springLen * b3Vec3_axisY;

		raytranslation = b3Body_GetWorldVector(bid, raytranslation);
		

		b3QueryFilter skipTeamFilter = { 1, ~2u };
		b3RayResult result = b3World_CastRayClosest(playground->_worldId, rayorigin, raytranslation, skipTeamFilter);

		b3Vec3 force = b3Vec3{ 0 };
		b3Vec3 springVel = b3Vec3{ 0 };

		b3Vec3 springforce = b3Vec3{ 0 };

		b3Vec3 frictionforce = b3Vec3{ 0 };

		b3Vec3 torqueforce = b3Vec3{ 0 };



		if (this->_accelerating) {

			this->_wheels[i]->_speed += this->_torqueCurve.evaluate(this->_wheels[i]->_speed / _maxSpeed) * this->_torque * playground->_targetDeltaTime;

			//torqueforce = _torqueCurve.evaluate(wheelVel.z / _maxSpeed) * _torque * b3Vec3_axisZ;
			//vecToWheel(&torqueforce, -_wheels[i]->_angle);
			//torqueforce = b3Body_GetWorldVector(bid, torqueforce);
		}
		else {
			float basicFriction = 10.0f * playground->_targetDeltaTime;
			if (this->_wheels[i]->_speed > 0.0f) this->_wheels[i]->_speed -= Clamp(basicFriction, 0.0f, this->_wheels[i]->_speed);
			else this->_wheels[i]->_speed += Clamp(basicFriction, 0.0f, -this->_wheels[i]->_speed);
		}
		

		float maxVelFric = 30.0f * playground->_targetDeltaTime;
		
		

		if (this->_braking && i >= 2) {
			this->_wheels[i]->_speed = 0.0f;
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

			this->_wheels[i]->_prevHeight = this->_wheels[i]->_springLen;

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
			float scalarSpringForce = b3Dot(springforce, result.normal);
			springforce = scalarSpringForce * result.normal;


			//_wheels[i]->_speed = wheelVel.z; //zVecFromWheel(&wheelVel, _wheels[i]->_angle);


			float diff = _wheels[i]->_speed - wheelVel.z;
			/*
			diff = Clamp(diff, -maxVelFric, maxVelFric);

			_wheels[i]->_speed -= diff;
			
			torqueforce = diff * playground->_targetFPS * b3Vec3_axisZ * bodyMass * 0.25f;
			vecToWheel(&torqueforce, -_wheels[i]->_angle);
			torqueforce = b3Body_GetWorldVector(bid, torqueforce);
			*/
			
			
			//if (!_accelerating) _wheels[i]->_speed -= diff;

			float pifagor_2 = wheelVel.x * wheelVel.x + diff * diff;
			float pifagor = sqrtf(pifagor_2);
			if (pifagor_2 >= maxVelFric * maxVelFric) {
				this->_wheels[i]->_sliding = true;
				float factor = maxVelFric / pifagor;
				wheelVel.x *= factor;
				diff *= factor;
			}
			else {
				this->_wheels[i]->_sliding = false;
			}

			//if (_accelerating);
			this->_wheels[i]->_speed -= diff;

			frictionforce = -wheelVel.x * (float)playground->_targetFPS * b3Vec3_axisX * bodyMass * 0.25f;
			vecToWheel(&frictionforce, -this->_wheels[i]->_angle);
			frictionforce = b3Body_GetWorldVector(bid, frictionforce);
			
			

			torqueforce = diff * (float)playground->_targetFPS * b3Vec3_axisZ * bodyMass * 0.25f;
			vecToWheel(&torqueforce, -this->_wheels[i]->_angle);
			torqueforce = b3Body_GetWorldVector(bid, torqueforce);
			
			

			force += springforce;

			force += frictionforce;

			force += torqueforce;


			b3Body_ApplyForce(bid, force, result.point, true);


			this->_wheels[i]->_prevHeight = this->_wheels[i]->_springLen * result.fraction;
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
	
	this->_accelerating = false;
	this->_braking = false;
}




void Particle::update(Playground* playground)
{
	if (this->_onRemove) return;

	this->_timeSeconds += playground->_targetDeltaTime;

	this->_pos += _linVel* playground->_targetDeltaTime;
	this->_scale += _scaleVel * playground->_targetDeltaTime;
	this->_alpha += _alphaVel * playground->_targetDeltaTime;

	if (this->_timeSeconds > this->_lifeTimeSeconds) this->_onRemove = true;
}

void Particle::draw(Playground* playground)
{
	if (this->_onRemove) return;

	rlDisableDepthMask();
	BeginBlendMode(BLEND_ADDITIVE);
	

	DrawBillboardPro(playground->_camera._cam, playground->_textures[this->_texId],
		Rectangle{0.0f, 0.0f, (float)playground->_textures[_texId].width, (float)playground->_textures[_texId].height},
		this->_pos, playground->_camera._cam.up,
		Vector2{ this->_scale.x, this->_scale.z }, Vector2{ this->_scale.x*0.5f, this->_scale.z*0.5f }, this->_anVel* this->_timeSeconds, Color{255, 255, 255, (unsigned char)b3ClampInt((int)this->_alpha, 0, 255)});
	EndBlendMode();
	rlEnableDepthMask();
}


