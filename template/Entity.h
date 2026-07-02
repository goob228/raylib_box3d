#ifndef ENTITY_H
#define ENTITY_H

#include "Object.h"



class Entity : public Object
{
public:
	Entity() : Object() { _type = OBJ_ENTITY; _alive = true; };

protected:

	

	int _health = 100;
};

#endif