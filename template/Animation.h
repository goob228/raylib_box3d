#ifndef ANIMATION_H
#define ANIMATION_H

#define CURVE_MAX_FLOAT 32

class LookUpCurve {

public:


	float evaluate(float x);


	int len = 0;

	float val[CURVE_MAX_FLOAT] = { 0 };
};




#endif