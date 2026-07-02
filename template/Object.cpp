#include "Object.h"
#include "Playground.h"

#include <box3d/box3d.h>


void Object::update(Playground* playground)
{
	if (_type != OBJ_STATIC) {
		if (_physId != 0) {
			b3BodyId b3id = playground->_bodies[_physId];
			b3Vec3 position = b3Body_GetPosition(b3id);
			b3Quat rotation = b3Body_GetRotation(b3id);
			_pos.x = position.x;
			_pos.y = position.y;
			_pos.z = position.z;
			_rot.x = rotation.v.x;
			_rot.y = rotation.v.y;
			_rot.z = rotation.v.z;
			_rot.w = rotation.s;
		}
		updateMatrix();
	}
}

void Object::updateMatrix()
{
	_model.transform = MatrixMultiply(MatrixMultiply(QuaternionToMatrix(_rot), MatrixScale(_scale.x, _scale.y, _scale.z)), MatrixTranslate(_pos.x, _pos.y, _pos.z));
}

void Object::draw()
{
	DrawModel(_model, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
}