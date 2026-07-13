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


float LookUpCurve::evaluate(float x)
{
	if (x <= 0.0f) return val[0];

	if (x >= 1.0f) return val[len-1];

	float index = x * (len - 1);

	int leftindex = (int)index;
	int rightindex = leftindex + 1;

	float frac = index - leftindex;

	return val[leftindex] * (1.0f - frac) + val[rightindex] * frac;
}


void Spring::evaluate()
{
	if (fabsf(damping - 1) < 0.001) {
		float p = 4.0f * PI * hertz;
		float k = -p / 2.0f;

		float C1 = startPosition;
		float C2 = startVelocity - C1 * k;

		position = expf(k * elapsedTime) * (C1 + C2 * elapsedTime);

		velocity = C1 * k * expf(k * elapsedTime) + C2 * expf(k * elapsedTime) + C2 * elapsedTime * k * expf(k * elapsedTime);

		return;
	}

	if (damping < 1.0f) {
		float p = 4.0f * PI * damping * hertz;
		float sq_q = (2.0f * PI * hertz);
		float q = sq_q * sq_q;

		float alpha = -p / 2.0f;
		float beta = sqrtf(4.0f * q - p * p) / 2.0f;

		float C1 = startPosition;
		float C2 = (startVelocity - C1 * alpha) / beta;

		position = expf(alpha * elapsedTime) * (C1 * cosf(beta * elapsedTime) + C2 * sinf(beta * elapsedTime));

		velocity = expf(alpha * elapsedTime) *
			((C2 * alpha - C1 * beta) * sinf(beta * elapsedTime) + (C2 * beta + C1 * alpha) * cosf(beta * elapsedTime));



		return;
	}


	if (damping > 1.0f) {
		float p = 4.0f * PI * damping * hertz;
		float sq_q = (2.0f * PI * hertz);
		float q = sq_q * sq_q;
		float D = p * p - 4.0f * q;
		float k1 = (-p + sqrtf(D)) / 2.0f;
		float k2 = (-p - sqrtf(D)) / 2.0f;

		float C1 = (startVelocity - startPosition * k2) / (k1 - k2);
		float C2 = startPosition - C1;
		
		position = C1 * expf(k1 * elapsedTime) + C2 * expf(k2 * elapsedTime);

		velocity = C1 * k1 * expf(k1 * elapsedTime) + C2 * k2 * expf(k2 * elapsedTime);


		return;
	}



}

void Spring::update(float deltaTimeSeconds)
{
	elapsedTime += deltaTimeSeconds;
}



void Spring::setPos(float pos)
{
	startPosition = pos - targetPosition;
	elapsedTime = 0.0f;
}

void Spring::setTargetPos(float targetPos)
{
	evaluate();
	startPosition = position + targetPosition - targetPos;
	targetPosition = targetPos;
	startVelocity = velocity;
	elapsedTime = 0.0f;
}

void Spring::setDamping(float damp)
{
	damping = damp;
}

void Spring::setHertz(float frequency)
{
	hertz = frequency;
}


float Spring::getPos()
{
	evaluate();
	return position + targetPosition;
}
