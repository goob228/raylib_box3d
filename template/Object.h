#ifndef OBJECT_H
#define OBJECT_H

#include <stdint.h>

#include <box3d/box3d.h>
#include <raylib.h>
#include <raymath.h>

#include "Resource.h"
#include "EventHandler.h"


typedef struct {
    b3Pos prev;
    b3Pos curr;
} Position;

typedef struct {
    b3Quat prev;
    b3Quat curr;
} Rotater;

typedef struct {
    Matrix transform;
	Quaternion rot;
	Vector3 pos;
	Vector3 scale;
    Resource_key texres;
	Resource_key modelres;
} Drawable;

typedef struct DrawFunc {
	void (*draw)(struct Drawable* drawable);
} DrawFunc;

typedef struct {
	EventHandler* ptr;
} EventPtr;

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




typedef struct Object
{


	void (*update)(struct Object* self);
	void (*setParent)(struct Object* self, struct Object* object);
	uint8_t data[MAX_OBJECT_DATA];
	struct Object* parent;
	Matrix transform;
	Quaternion rot;
	Vector3 pos;
	Vector3 scale;
	ObjectType type;
	int physId;
	Resource_key texres;
	Resource_key modelres;
	bool onRemove;
	bool alive;
	

} Object;

void ob_update(struct Object* self);

void ob_updateMatrix(struct Object* self);

void ob_updateMatrix2(struct Object* self, Position* pos, Rotater* rot);

void ob_updateMatrix3(Drawable* drawable, Position* pos, Rotater* rot);

void ob_updateMatrix4(Drawable* drawable);

void ob_draw(struct Object* self);

void ob_setParent(struct Object* self, struct Object* obj);



#endif