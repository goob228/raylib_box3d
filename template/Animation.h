#ifndef ANIMATION_H
#define ANIMATION_H

#define CURVE_MAX_FLOAT 32

float easeOutBack(float x);


class LookUpCurve {

public:


	float evaluate(float x);


	int len = 0;

	float val[CURVE_MAX_FLOAT] = { 0 };
};




class Spring {

public:


	void update(float deltaTimeSeconds);


	void setPos(float pos);
	void setTargetPos(float targetPos);
	void setDamping(float damp);
	void setHertz(float frequency);
	void evaluate();

	float getPos();
	float getVelocity();

	float position = 0.0f;
	float startPosition = 0.0f;
	float velocity = 0.0f;
	float startVelocity = 0.0f;
	float hertz = 1.0f;
	float damping = 1.0f;
	float targetPosition = 0.0f;
	float elapsedTime = 0.0f;

	bool used = false;

};



#endif