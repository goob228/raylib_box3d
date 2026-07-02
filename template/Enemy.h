#ifndef ENEMY_H
#define ENEMY_H

#include "Entity.h"

typedef enum {
	AI_IDLE,
	AI_ATTACKING,
	AI_SEARCHING,
} AIState;



class Enemy : public Entity
{
public:

	Enemy() : Entity() { _type = OBJ_ENEMY; _alive = true; };

protected:

	AIState _aiState = AI_IDLE;
	
};

#endif