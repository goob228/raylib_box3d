#ifndef ANIMATION_H
#define ANIMATION_H

#define CURVE_MAX_FLOAT 32


#include <stdbool.h>



float easeOutBack(float x);


typedef struct LookUpCurve {

	float (*evaluate)(struct LookUpCurve* self, float x);

	int len;
	float val[CURVE_MAX_FLOAT];

} LookUpCurve;



typedef struct Spring {


	void (*update)(struct Spring* self, float deltaTimeSeconds);


	void (*setPos)(struct Spring* self, float pos);
	void (*setTargetPos)(struct Spring* self, float targetPos);
	void (*setDamping)(struct Spring* self, float damp);
	void (*setHertz)(struct Spring* self, float frequency);
	void (*evaluate)(struct Spring* self);

	float (*getPos)(struct Spring* self);
	float (*getVelocity)(struct Spring* self);

	float position;
	float startPosition;
	float velocity;
	float startVelocity;
	float hertz;
	float damping;
	float targetPosition;
	float elapsedTime;
	bool used;

} Spring;


LookUpCurve curve_create();

Spring spring_create();



#endif