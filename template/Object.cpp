#include "Object.h"
#include "Playground.h"

#include <box3d/box3d.h>
#include <rlgl.h>


void Object::update(Playground* playground)
{
	if (_type != OBJ_STATIC && _type != OBJ_OBSTACLE) {
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
	_transform = MatrixMultiply(MatrixMultiply(MatrixScale(_scale.x, _scale.y, _scale.z), QuaternionToMatrix(_rot)), MatrixTranslate(_pos.x, _pos.y, _pos.z));

	if (_parent) {
		_transform = MatrixMultiply(_transform, _parent->_transform);
	}

}

void Object::draw(Playground* playground)
{
	playground->_models[_modelId].transform = _transform;
	playground->_models[_modelId].materials[0].maps[MATERIAL_MAP_ALBEDO].texture = playground->_textures[_texId];
	DrawModel(playground->_models[_modelId], Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
}



void Object::setParent(Object* obj)
{
	_parent = obj;
}