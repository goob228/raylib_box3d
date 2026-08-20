#ifndef OBJECT_H
#define OBJECT_H

#include <stdint.h>

#include <raylib.h>
#include <raymath.h>



typedef enum 
{
	OBJ_NONE,
	OBJ_EMPTY,
	OBJ_STATIC,
	OBJ_OBSTACLE,
	OBJ_PROP,
	OBJ_ENTITY,
	OBJ_PLAYER,
	OBJ_ENEMY,
	OBJ_PROJECTILE,
	OBJ_ITEM,
	OBJ_LEN
} ObjectType;

#define MAX_OBJECT_DATA 1024




typedef struct Playground Playground;


typedef struct Object
{

	void (*update)(struct Object* self, Playground* playground);
	void (*updateMatrix)(struct Object* self);
	void (*draw)(struct Object* self, Playground* playground);
	void (*setParent)(struct Object* self, struct Object* object);
	uint8_t data[MAX_OBJECT_DATA];
	struct Object* parent;
	Matrix transform;
	Quaternion rot;
	Vector3 pos;
	Vector3 scale;
	ObjectType type;
	int physId;
	int texId;
	int modelId;
	bool onRemove;
	bool alive;
	

} Object;

void ob_update(struct Object* self, Playground* playground);

void ob_updateMatrix(struct Object* self);

void ob_draw(struct Object* self, Playground* playground);

void ob_setParent(struct Object* self, struct Object* obj);



#endif