#include "Animation.h"

#include <raylib.h>
#include <raymath.h>

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