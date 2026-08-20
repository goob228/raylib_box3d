#include "Prefabs.h"

#include <math.h>


#include "Object.h"
#include "Camera.h"

#include <box3d/box3d.h>
#include <rlgl.h>

#include "Playground.h"
#include "EventHandler.h"
#include "Animation.h"





Object* wheel_create(Object* object)
{
	Object* wheel = object;

	wheel->update = (&wheel_update);

	WheelData* wheeldata = (WheelData*)wheel->data;

	wheeldata->defaultPos = (b3Vec3){ wheel->pos.x, wheel->pos.y, wheel->pos.z };
	wheeldata->springLen = 0.0f;
	wheeldata->prevHeight = 0.0f;
	wheeldata->angle = 0.0f;
	wheeldata->weight = 60.0f;

	wheeldata->speed = 0.0f;
	wheeldata->radius = 0.45f;
	wheeldata->YZangle = 0.0f;


	return wheel;
}

void wheel_update(Object* obj, Playground* playground)
{
	Object* self = obj;
	WheelData* wheeldata = (WheelData*)self->data;
	if (wheeldata->car) {
		self->pos = (Vector3){ wheeldata->defaultPos.x, 
				wheeldata->defaultPos.y - wheeldata->prevHeight + wheeldata->radius, wheeldata->defaultPos.z};
		wheeldata->YZangle += wheeldata->speed / wheeldata->radius * playground->targetDeltaTime;
		wheeldata->YZangle = fmodf(wheeldata->YZangle, 2 * PI);
		self->rot = QuaternionFromEuler(wheeldata->YZangle, wheeldata->angle, 0.0f);
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


	cardata->springLen = 1.1f;
	cardata->springStiffness = 800.0f;
	cardata->springDamping = 0.9f;
	cardata->tireFriction = 0.8f;
	cardata->accelerating = false;
	cardata->braking = false;
	cardata->torqueCurve = curve_create();
	cardata->torque = 40.0f;
	cardata->maxSpeed = 70.0f;
	cardata->wheelCount = 0;

	
	
	
	cardata->springStiffness = 10.0f * b3Body_GetMass(playground->bodies[car->physId]) * 20.0f * 0.25f;


	b3Vec3 masscen = b3Body_GetLocalCenter(playground->bodies[car->physId]);
 
	cardata->springDamping = 5.0f;


	cardata->torqueCurve.len = 11;
	float mv[] = { 0.5f, 0.6f, 0.8f, 0.95f, 1.0f ,1.0f ,1.0f, 0.9f, 0.5f, 0.2f, 0.0f };
	for (int i = 0; i < cardata->torqueCurve.len; i++) {
		cardata->torqueCurve.val[i] = mv[i];
	}


	return car;
}


void car_steer(Object* self, float angleDeg)
{
	CarData* cardata = (CarData*)self->data;
	if (cardata->wheelCount >= 2) {
		((WheelData*)cardata->wheels[0]->data)->angle = angleDeg * DEG2RAD;
		((WheelData*)cardata->wheels[1]->data)->angle = angleDeg * DEG2RAD;
	}
}



void car_update(Object* obj, Playground* playground)
{	
	Object* self = (Object*)obj;
	CarData* cardata = (CarData*)self->data;
	cardata->og_update(obj, playground);

	b3BodyId bid = playground->bodies[self->physId];

	float bodyMass = b3Body_GetMass(bid);

	b3WorldTransform transform = b3Body_GetTransform(bid);

	b3Pos pos = b3Body_GetPosition(bid);

	b3Vec3 linVel = b3Body_GetLinearVelocity(bid);

	b3Vec3 anVel =  b3Body_GetAngularVelocity(bid);

	b3Vec3 wheelVel = { 0 };


	for (int i = 0; i < cardata->wheelCount; i++) {
		if (!cardata->wheels[i]) continue;
		wheelVel = b3Body_GetLocalPointVelocity(bid, ((WheelData*)cardata->wheels[i]->data)->defaultPos);
		wheelVel = b3Body_GetLocalVector(bid, wheelVel);
		vecToWheel(&wheelVel, ((WheelData*)cardata->wheels[i]->data)->angle);
		b3Pos rayorigin = b3Body_GetWorldPoint(bid, ((WheelData*)cardata->wheels[i]->data)->defaultPos);
		b3Vec3 raytranslation = (b3Vec3){0.0f, -((WheelData*)cardata->wheels[i]->data)->springLen, 0.0f };

		raytranslation = b3Body_GetWorldVector(bid, raytranslation);
		

		b3QueryFilter skipTeamFilter = { 1, ~2u };
		b3RayResult result = b3World_CastRayClosest(playground->worldId, rayorigin, raytranslation, skipTeamFilter);

		b3Vec3 force = (b3Vec3){ 0 };
		b3Vec3 springVel = (b3Vec3){ 0 };

		b3Vec3 springforce = (b3Vec3){ 0 };

		b3Vec3 frictionforce = (b3Vec3){ 0 };

		b3Vec3 torqueforce = (b3Vec3){ 0 };



		if (cardata->accelerating) {

			((WheelData*)cardata->wheels[i]->data)->speed += cardata->torqueCurve.evaluate(&cardata->torqueCurve, ((WheelData*)cardata->wheels[i]->data)->speed / cardata->maxSpeed) * cardata->torque * playground->targetDeltaTime;

			//torqueforce = _torqueCurve.evaluate(wheelVel.z / _maxSpeed) * _torque * b3Vec3_axisZ;
			//vecToWheel(&torqueforce, -_wheels[i]->_angle);
			//torqueforce = b3Body_GetWorldVector(bid, torqueforce);
		}
		else {
			float basicFriction = 10.0f * playground->targetDeltaTime;
			if (((WheelData*)cardata->wheels[i]->data)->speed > 0.0f) 
				((WheelData*)cardata->wheels[i]->data)->speed -= Clamp(basicFriction, 0.0f, ((WheelData*)cardata->wheels[i]->data)->speed);
			else 
				((WheelData*)cardata->wheels[i]->data)->speed += Clamp(basicFriction, 0.0f, -((WheelData*)cardata->wheels[i]->data)->speed);
		}
		

		float maxVelFric = 30.0f * playground->targetDeltaTime;
		
		

		if (cardata->braking && i >= 2) {
			((WheelData*)cardata->wheels[i]->data)->speed = 0.0f;
			//if (_wheels[i]->_speed > 0.0f) _wheels[i]->_speed -= Clamp(maxVelFric, 0.0f, _wheels[i]->_speed);
			//else _wheels[i]->_speed += Clamp(maxVelFric, 0.0f, -_wheels[i]->_speed);
		}


		if (result.hit == false) {
			playground->lines[i * 2 + 0].x = rayorigin.x;
			playground->lines[i * 2 + 0].y = rayorigin.y;
			playground->lines[i * 2 + 0].z = rayorigin.z;

			playground->lines[i * 2 + 1].x = rayorigin.x + raytranslation.x;
			playground->lines[i * 2 + 1].y = rayorigin.y + raytranslation.y;
			playground->lines[i * 2 + 1].z = rayorigin.z + raytranslation.z;

			((WheelData*)cardata->wheels[i]->data)->prevHeight = ((WheelData*)cardata->wheels[i]->data)->springLen;

		}
		else {
			playground->lines[i * 2 + 0].x = rayorigin.x;
			playground->lines[i * 2 + 0].y = rayorigin.y;
			playground->lines[i * 2 + 0].z = rayorigin.z;

			playground->lines[i * 2 + 1].x = result.point.x;
			playground->lines[i * 2 + 1].y = result.point.y;
			playground->lines[i * 2 + 1].z = result.point.z;

			springVel = (b3Vec3){ 0.0f, wheelVel.y, 0.0f };
			springVel = b3Body_GetWorldVector(bid, springVel);
			float spring_factor_pid = ((result.fraction - 1.0f) * cardata->springStiffness);
			springforce = b3MulSub(b3Mul(raytranslation, (b3Vec3){ spring_factor_pid , spring_factor_pid,
											spring_factor_pid}), cardata->springDamping, springVel);
			float scalarSpringForce = b3Dot(springforce, result.normal);
			springforce = b3Mul(result.normal, (b3Vec3) { scalarSpringForce, scalarSpringForce
			, scalarSpringForce
			}); // scalarSpringForce * result.normal;

			float diff = ((WheelData*)cardata->wheels[i]->data)->speed - wheelVel.z;

			float pifagor_2 = wheelVel.x * wheelVel.x + diff * diff;
			float pifagor = sqrtf(pifagor_2);
			if (pifagor_2 >= maxVelFric * maxVelFric) {
				((WheelData*)cardata->wheels[i]->data)->sliding = true;
				float factor = maxVelFric / pifagor;
				wheelVel.x *= factor;
				diff *= factor;
			}
			else {
				((WheelData*)cardata->wheels[i]->data)->sliding = false;
			}

			//if (_accelerating);
			((WheelData*)cardata->wheels[i]->data)->speed -= diff;

			frictionforce = (b3Vec3){ -wheelVel.x * (float)playground->targetFPS * bodyMass * 0.25f, 0.0f, 0.0f }; //-wheelVel.x * (float)playground->_targetFPS * b3Vec3_axisX * bodyMass * 0.25f;
			vecToWheel(&frictionforce, -((WheelData*)cardata->wheels[i]->data)->angle);
			frictionforce = b3Body_GetWorldVector(bid, frictionforce);
			
			float friction_force_factor = b3Dot(frictionforce, result.normal);

			frictionforce = b3MulSub(frictionforce, friction_force_factor, result.normal);

			torqueforce = (b3Vec3){ 0.0f, 0.0f, diff * (float)playground->targetFPS * bodyMass * 0.25f };
				//diff * (float)playground->_targetFPS * b3Vec3_axisZ * bodyMass * 0.25f;
			vecToWheel(&torqueforce, -((WheelData*)cardata->wheels[i]->data)->angle);
			torqueforce = b3Body_GetWorldVector(bid, torqueforce);
			
			float torque_force_factor = b3Dot(torqueforce, result.normal);

			torqueforce = b3MulSub(torqueforce, torque_force_factor, result.normal);

			force = (b3Vec3){	springforce.x + frictionforce.x + torqueforce.x,
								springforce.y + frictionforce.y + torqueforce.y,
								springforce.z + frictionforce.z + torqueforce.z };

			b3Body_ApplyForce(bid, force, result.point, true);


			((WheelData*)cardata->wheels[i]->data)->prevHeight = ((WheelData*)cardata->wheels[i]->data)->springLen * result.fraction;
		}
		float factor1 = 10.0f;


		playground->lines[8 + i * 2 + 0].x = rayorigin.x;
		playground->lines[8 + i * 2 + 0].y = rayorigin.y;
		playground->lines[8 + i * 2 + 0].z = rayorigin.z;
		
		playground->lines[8 + i * 2 + 1].x = rayorigin.x + springforce.x * factor1 / cardata->springStiffness;
		playground->lines[8 + i * 2 + 1].y = rayorigin.y + springforce.y * factor1 / cardata->springStiffness;
		playground->lines[8 + i * 2 + 1].z = rayorigin.z + springforce.z * factor1 / cardata->springStiffness;

		float factor2 = 0.1f;
		
		playground->lines[16 + i * 2 + 0].x = rayorigin.x;
		playground->lines[16 + i * 2 + 0].y = rayorigin.y;
		playground->lines[16 + i * 2 + 0].z = rayorigin.z;

		playground->lines[16 + i * 2 + 1].x = rayorigin.x + frictionforce.x * factor2 / ((WheelData*)cardata->wheels[i]->data)->weight;
		playground->lines[16 + i * 2 + 1].y = rayorigin.y + frictionforce.y * factor2 / ((WheelData*)cardata->wheels[i]->data)->weight;// -weight * 0.25;
		playground->lines[16 + i * 2 + 1].z = rayorigin.z + frictionforce.z * factor2 / ((WheelData*)cardata->wheels[i]->data)->weight;
		
	}
	
	cardata->accelerating = false;
	cardata->braking = false;
}

static bool MoverFilterCallback( b3ShapeId shapeId, void* context )
{
	Object* self = (Object*)context;
	CharacterData* data = (CharacterData*)self->data;
	for ( int i = 0; i < data->ignoreCount; ++i )
	{
		if ( B3_ID_EQUALS( shapeId, data->ignoreShapeIds[i] ) )
		{
			return false;
		}
	}

	return true;
}

static bool PlaneResultFcn( b3ShapeId shapeId, const b3PlaneResult* planeResults, int planeCount, void* context )
{
	if ( MoverFilterCallback( shapeId, context ) == false )
	{
		// ignore these planes but continue looking for more
		return true;
	}

	Object* self = (Object*)context;
	CharacterData* data = (CharacterData*)self->data;

	float maxPush = FLT_MAX;
	bool clipVelocity = true;
	MoverShapeUserData* userData = (MoverShapeUserData*)b3Shape_GetUserData( shapeId );
	if ( userData != 0 )
	{
		maxPush = userData->maxPush;
		clipVelocity = userData->clipVelocity;
	}

	for ( int i = 0; i < planeCount && data->planeCount < PLANE_CAPACITY; ++i )
	{
		//assert( b3IsValidPlane( planeResults[i].plane ) );
		data->planes[data->planeCount] = (b3CollisionPlane){
			planeResults[i].plane,
			maxPush,
			0.0f,
			clipVelocity
		};
		data->planeExtras[data->planeCount] = (struct PlaneExtra){
			b3OffsetPos( data->trans.p, planeResults[i].point ),
			shapeId,
		};
		data->planeCount += 1;
	}

	return true;
}



void character_solveMove(Object* obj, float timeStep, b3Vec3 forward, b3Vec3 right, b3Vec2 throttle, bool clipVelocity)
{
	Object* self = obj;
	CharacterData* data = (CharacterData*)self->data;

	float speed = b3Length(data->velocity);
	if (speed < data->minSpeed){
		data->velocity.x = 0.0f;
		data->velocity.z = 0.0f;
	} else if (data->onGround) {
		float control = speed < data->stopSpeed ? data->stopSpeed : speed;

		float drop = control * data->friction * timeStep;
		float newSpeed = b3MaxFloat(0.0f, speed - drop);
		float ratio = newSpeed / speed;
		data->velocity.x *= ratio;
		data->velocity.z *= ratio;
	}
	float maxSpeed = data->sprint ? 2.0f * data->maxSpeed: data->maxSpeed;
	float temp_mstx = maxSpeed*throttle.x;
	b3Vec3 desiredVelocity = b3MulAdd((b3Vec3){temp_mstx * forward.x, temp_mstx * forward.y, temp_mstx * forward.z}, 
												maxSpeed*throttle.y, right);

	float desiredSpeed;
	b3Vec3 desiredDirection = b3GetLengthAndNormalize(&desiredSpeed, desiredVelocity);

	if (desiredSpeed > maxSpeed) {
		desiredVelocity.x *= maxSpeed / desiredSpeed;
		desiredVelocity.y *= maxSpeed / desiredSpeed;
		desiredVelocity.z *= maxSpeed / desiredSpeed;
	}

	

	//Accelerate

	if (data->onGround) {
		data->velocity.y = 0.0f;
	}
	

	float currentSpeed = b3Dot(data->velocity, desiredDirection);
	float addSpeed = maxSpeed - currentSpeed;
	if (addSpeed > 0.0f) {
		float accelSpeed = data->accelerate * maxSpeed * timeStep;
		if (accelSpeed > addSpeed) {
			accelSpeed = addSpeed;
		}
		data->velocity = b3MulAdd(data->velocity, accelSpeed, desiredDirection);
	}
	
	data->velocity.y -= data->gravity * timeStep;

	b3WorldId worldId = data->playground->worldId;
	
	float pogoRestLength = 1.0f * data->capsule.radius;
	float rayLength = pogoRestLength + data->capsule.radius;
	data->trans.p = (b3Pos){self->pos.x, self->pos.y, self->pos.z};
	b3Pos rayOrigin = b3TransformWorldPoint(data->trans, data->capsule.center1);
	b3Vec3 rayTranslation = (b3Vec3){0.0f, -rayLength, 0.0f};
	b3QueryFilter skipTeamFilter = { 1, ~2u };
	b3RayResult rayResult = b3World_CastRayClosest( worldId, rayOrigin, rayTranslation, skipTeamFilter );

	

	// After gravity was applied, disable pogo when still moving up.
	// Avoids getting pulled back to the ground when jumping.
	bool suppressPogo = data->velocity.y > 0.0f;

	if (rayResult.hit == false || suppressPogo) {
		data->onGround = false;
		data->pogoVelocity = 0.0f;
	} else {
		data->onGround = true;
		float pogoCurrentLength = rayResult.fraction * rayLength;

		float zeta = 0.7f;
		float hertz = 4.0f;
		float omega = 2.0f * B3_PI * hertz;
		float omegaH = omega * timeStep;
		float prevPogoVelocity = data->pogoVelocity;
		data->pogoVelocity = ( data->pogoVelocity - omega * omegaH * ( pogoCurrentLength - pogoRestLength ) ) /
						 ( 1.0f + 2.0f * zeta * omegaH + omegaH * omegaH );
		if (b3Shape_IsValid(rayResult.shapeId)) {
			b3BodyId bodyId = b3Shape_GetBody(rayResult.shapeId);
			
			b3BodyType bodyType = b3Body_GetType(bodyId);

			if (bodyType == b3_dynamicBody) {
				b3Body_ApplyForce(bodyId,  b3MulSV( -data->mass*data->gravity,  rayResult.normal), rayResult.point, true);
				b3Body_ApplyLinearImpulse(bodyId,  b3MulSV( -data->mass*(data->pogoVelocity-prevPogoVelocity),  rayResult.normal), rayResult.point, true);
			}
		}
			
	}

	b3Pos startPosition = data->trans.p;
	b3Pos target = b3Add(b3MulAdd(data->trans.p, timeStep, data->velocity), (b3Vec3){0.0f, timeStep*data->pogoVelocity, 0.0f});

	// Want the mover to collide with allies
	b3QueryFilter moverFilter = { .categoryBits = 1, .maskBits = ~0u, .id = 1, .name = "mover_collide" };

	// The cast should ignore allies
	b3QueryFilter castFilter = { .categoryBits = 1, .maskBits = ~2u, .id = 1, .name = "mover_cast" };

	data->totalIterations = 0;
	float tolerance = 0.01f;

	for (int iteration = 0; iteration < 5; iteration++) 
	{
		data->planeCount = 0;
		b3Capsule mover;
		mover.center1 = data->capsule.center1;
		mover.center2 = data->capsule.center2;
		mover.radius = data->capsule.radius;

		b3World_CollideMover(worldId, data->trans.p, &mover, moverFilter, PlaneResultFcn, self);

		b3Vec3 targetDelta = b3Sub(target, data->trans.p);
		b3PlaneSolverResult result = b3SolvePlanes(targetDelta, data->planes, data->planeCount);

		data->totalIterations += result.iterationCount;

		b3Vec3 delta = result.delta;

		float fraction = b3World_CastMover(worldId, data->trans.p, &mover, delta, castFilter,  MoverFilterCallback, self);

		delta.x *= fraction;
		delta.y *= fraction;
		delta.z *= fraction;
		data->trans.p = b3Add(data->trans.p, delta);

		if (b3LengthSquared(delta) < tolerance * tolerance) {
			break;
		}

	}
	float invMassA = 1.0f/data->mass;
	for (int i = 0; i < data->planeCount; i++){
		b3BodyId bodyId = b3Shape_GetBody(data->planeExtras[i].shapeId);
		b3BodyType bodyType = b3Body_GetType(bodyId);

		if (bodyType != b3_dynamicBody) {
			continue;
		}

		b3Pos point = data->planeExtras[i].point;
		b3Vec3 normal = b3Neg(data->planes[i].plane.normal);

		
		float invMassB = b3Body_GetInverseMass(bodyId);
		b3Matrix3 invIB = b3Body_GetWorldInverseRotationalInertia( bodyId );

		b3Pos pB = b3Body_GetWorldCenter( bodyId );
		b3Vec3 rB = b3SubPos( point, pB );

		b3Vec3 rnB = b3Cross( rB, normal );
		float kNormal = invMassA + invMassB + b3Dot( rnB, b3MulMV( invIB, rnB ) );
		float normalMass = kNormal > 0.0f ? 1.0f / kNormal : 0.0f;

		b3Vec3 vB = b3Body_GetLinearVelocity( bodyId );
		b3Vec3 omegaB = b3Body_GetAngularVelocity( bodyId );
		b3Vec3 vrB = b3Add( vB, b3Cross( omegaB, rB ) );
		float vn = b3Dot( b3Sub( vrB, data->velocity ), normal );
		float impulse = b3MaxFloat( -normalMass * vn, 0.0f );

		b3Vec3 P = b3MulSV( impulse, normal );
		data->velocity = b3MulSub( data->velocity, invMassA, P );

		b3Body_ApplyLinearImpulse( bodyId, P, point, true );
		//b3Body_ApplyForce(bodyId,  b3MulSV( data->mass*data->gravity, normal ), point, true);
	}

	if (clipVelocity) {
		// Using the velocity clipper can avoid picking up velocity from depenetration.
		// This allows the mover to avoid velocity from soft collision depenetration.
		data->velocity = b3ClipVector(data->velocity, data->planes, data->planeCount);
	} else if (timeStep > 0.0f) {
		// Using the position delta is more holistic and intuitive in some cases.
		data->velocity = b3Sub(data->trans.p, startPosition);
		float temp_factor = 1.0f / timeStep;
		data->velocity.x *= temp_factor;
		data->velocity.y *= temp_factor;
		data->velocity.z *= temp_factor;
	}
}

void character_update(Object* obj, Playground* playground)
{
	Object* self = obj;
	CharacterData* data = (CharacterData*)self->data;
	self->pos = data->trans.p;
	self->updateMatrix(self);
	if (!data->camera) return;
	CameraData* camdata = (CameraData*)data->camera->data;
	b3Vec2 throttle = { 0.0f, 0.0f };
	b3Vec3 forward = camdata->getForward(data->camera);
	b3Vec3 right = camdata->getRight(data->camera);
	right = b3Normalize(right);
	forward.y = 0.0f;
	forward = b3Normalize(forward);

	if (playground->eh.keys & EH_K_W) {
		throttle.x += 1.0f;
	}
	if (playground->eh.keys & EH_K_S) {
		throttle.x -= 1.0f;
	}
	if (playground->eh.keys & EH_K_A) {
		throttle.y += 1.0f;
	}
	if (playground->eh.keys & EH_K_D) {
		throttle.y -= 1.0f;
	}

	if ((playground->eh.keys & EH_K_SPACE) && data->onGround == true) {
		data->velocity.y = data->jumpSpeed;
		data->onGround = false;
	}
	if (data->onGround && (playground->eh.keys & EH_K_SHIFT)) {
		data->sprint = true;
	} else {
		data->sprint = false;
	}

	float hertz = playground->targetFPS;
	float timeStep = hertz > 0.0f ? 1.0f / hertz : 0.0f;

	character_solveMove(self, timeStep, forward, right, throttle, true);
	b3Pos position = self->pos;
}

void char_draw(struct Object* self, Playground* playground)
{	
	CharacterData* data = (CharacterData*)self->data;
	if (!data->camera) return;
	CameraData* camdata = (CameraData*)data->camera->data;
	if (camdata->type == CAM_FIRST_PERSON) return;
	playground->models[self->modelId].transform = self->transform;
	playground->models[self->modelId].materials[0].maps[MATERIAL_MAP_ALBEDO].texture = playground->textures[self->texId];
	DrawModel(playground->models[self->modelId], (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
}



Object* character_create(Object* object, Object* camera, Playground* playground)
{
	Object* self = object;
	CharacterData* data = (CharacterData*)self->data;
	
	data->capsule = (b3Capsule){ { 0.0f, -0.5f, 0.0f }, { 0.0f, 0.5f, 0.0f }, 0.3f };
	data->trans = b3Transform_identity;
	data->trans.p = (b3Vec3){self->pos.x, self->pos.y, self->pos.z};
	data->velocity = b3Vec3_zero;
	data->camera = camera;
	data->playground = playground;
	data->jumpSpeed = 10.0f;
	data->maxSpeed = 8.0f;
	data->minSpeed = 0.01f;
	data->stopSpeed = 1.0f;
	data->accelerate = 10.0f;
	data->friction = 4.0f;
	data->gravity = 20.0f;
	data->mass = 100.0f;
	data->onGround = false;
	data->sprint = false;

	data->planeCount = 0;
	data->totalIterations = 0;
	data->ignoreCount = 0;
	data->ignoreShapeIds = 0;

	data->pogoVelocity = 0.0f;
	
	self->update = character_update;
	self->draw = char_draw;
	

}