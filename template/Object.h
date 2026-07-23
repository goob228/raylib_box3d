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

#define MAX_OBJECT_DATA 256

#define OBJECT_FIELDS \
	void (*update)(struct Object* self, Playground* playground); \
	void (*updateMatrix)(struct Object* self); \
	void (*draw)(struct Object* self, Playground* playground); \
	void (*setParent)(struct Object* self, struct Object* object); \
	Matrix _transform; \
	Vector3 _pos; \
	Quaternion _rot; \
	Vector3 _scale; \
	bool _alive; \
	ObjectType _type; \
	struct Object* _parent; \
	int _physId; \
	int _texId; \
	int _modelId; \
	bool _onRemove;\
	uint8_t data[MAX_OBJECT_DATA];


typedef struct Playground Playground;

typedef struct Object
{

	OBJECT_FIELDS

} Object;

void ob_update(struct Object* self, Playground* playground);

void ob_updateMatrix(struct Object* self);

void ob_draw(struct Object* self, Playground* playground);

void ob_setParent(struct Object* self, struct Object* obj);



#endif