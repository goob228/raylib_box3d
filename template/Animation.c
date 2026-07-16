#include "Animation.h"

#include <raylib.h>
#include <raymath.h>


float easeOutBack(float x)
{
	float const c1 = 1.70158;
	float const c3 = c1 + 1.0f;
	float x_minus_one_2 = (x - 1) * (x - 1);

	return 1.0f + c3 * x_minus_one_2 * (x - 1) + c1 * x_minus_one_2;
}


float luc_evaluate(struct LookUpCurve* self, float x)
{
	if (x <= 0.0f) return self->val[0];

	if (x >= 1.0f) return self->val[self->len-1];

	float index = x * (self->len - 1);

	int leftindex = (int)index;
	int rightindex = leftindex + 1;

	float frac = index - leftindex;

	return self->val[leftindex] * (1.0f - frac) + self->val[rightindex] * frac;
}


void spring_evaluate(struct Spring* self)
{
	if (fabsf(self->damping - 1) < 0.001) {
		float p = 4.0f * PI * self->hertz;
		float k = -p / 2.0f;

		float C1 = self->startPosition;
		float C2 = self->startVelocity - C1 * k;

		self->position = expf(k * self->elapsedTime) * (C1 + C2 * self->elapsedTime);

		self->velocity = C1 * k * expf(k * self->elapsedTime) + C2 * expf(k * self->elapsedTime) + C2 * self->elapsedTime * k * expf(k * self->elapsedTime);

		return;
	}

	if (self->damping < 1.0f) {
		float p = 4.0f * PI * self->damping * self->hertz;
		float sq_q = (2.0f * PI * self->hertz);
		float q = sq_q * sq_q;

		float alpha = -p / 2.0f;
		float beta = sqrtf(4.0f * q - p * p) / 2.0f;

		float C1 = self->startPosition;
		float C2 = (self->startVelocity - C1 * alpha) / beta;

		self->position = expf(alpha * self->elapsedTime) * (C1 * cosf(beta * self->elapsedTime) + C2 * sinf(beta * self->elapsedTime));

		self->velocity = expf(alpha * self->elapsedTime) *
			((C2 * alpha - C1 * beta) * sinf(beta * self->elapsedTime) + (C2 * beta + C1 * alpha) * cosf(beta * self->elapsedTime));



		return;
	}


	if (self->damping > 1.0f) {
		float p = 4.0f * PI * self->damping * self->hertz;
		float sq_q = (2.0f * PI * self->hertz);
		float q = sq_q * sq_q;
		float D = p * p - 4.0f * q;
		float k1 = (-p + sqrtf(D)) / 2.0f;
		float k2 = (-p - sqrtf(D)) / 2.0f;

		float C1 = (self->startVelocity - self->startPosition * k2) / (k1 - k2);
		float C2 = self->startPosition - C1;
		
		self->position = C1 * expf(k1 * self->elapsedTime) + C2 * expf(k2 * self->elapsedTime);

		self->velocity = C1 * k1 * expf(k1 * self->elapsedTime) + C2 * k2 * expf(k2 * self->elapsedTime);


		return;
	}



}



void spring_update(struct Spring* self, float deltaTimeSeconds)
{
	self->elapsedTime += deltaTimeSeconds;
}



void spring_setPos(struct Spring* self, float pos)
{
	self->startPosition = pos - self->targetPosition;
	self->elapsedTime = 0.0f;
}

void spring_setTargetPos(struct Spring* self, float targetPos)
{
	self->evaluate(self);
	self->startPosition = self->position + self->targetPosition - targetPos;
	self->targetPosition = targetPos;
	self->startVelocity = self->velocity;
	self->elapsedTime = 0.0f;
}

void spring_setDamping(struct Spring* self, float damp)
{
	self->damping = damp;
}

void spring_setHertz(struct Spring* self, float frequency)
{
	self->hertz = frequency;
}


float spring_getPos(struct Spring* self)
{
	self->evaluate(self);
	return self->position + self->targetPosition;
}

LookUpCurve curve_create()
{
	LookUpCurve curve = { 0 };

	curve.len = 0;
	curve.evaluate = (&luc_evaluate);

	return curve;

}


Spring spring_create()
{
	Spring spring = { 0 };

	spring.position = 0.0f;
	spring.startPosition = 0.0f;
	spring.velocity = 0.0f;
	spring.startVelocity = 0.0f;
	spring.hertz = 1.0f;
	spring.damping = 1.0f;
	spring.targetPosition = 0.0f;
	spring.elapsedTime = 0.0f;

	spring.evaluate = (&spring_evaluate);
	spring.update = (&spring_update);
	spring.setDamping = (&spring_setDamping);
	spring.setHertz = (&spring_setHertz);
	spring.getPos = (&spring_getPos);
	spring.setTargetPos = (&spring_setTargetPos);
	spring.setPos = (&spring_setPos);

	spring.used = true;

	return spring;
}