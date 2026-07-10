#ifndef OBJECT_H
#define OBJECT_H


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

class Playground;

class Object
{
	friend Playground;
public:

	virtual void update(Playground* playground);

	void updateMatrix();

	virtual void draw(Playground* playground);

	void setParent(Object* object);
	

//protected:

	Matrix _transform = MatrixIdentity();

	Vector3 _pos = Vector3{ 0.0f, 0.0f, 0.0f };
	Quaternion _rot = QuaternionIdentity();
	Vector3 _scale = Vector3{ 1.0f, 1.0f, 1.0f };

	bool _alive = true;
	ObjectType _type = OBJ_NONE;

	Object* _parent;

	int _physId = 0;
	int _texId = 0;
	int _modelId = 0;

};



#endif