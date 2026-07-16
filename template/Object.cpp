#include "Object.h"
#include "Playground.h"

#include <box3d/box3d.h>
#include <rlgl.h>


void Object::update(Playground* playground)
{
	if (this->_type != OBJ_STATIC && this->_type != OBJ_OBSTACLE) {
		if (this->_physId != 0) {
			b3BodyId b3id = playground->_bodies[this->_physId];
			b3Vec3 position = b3Body_GetPosition(b3id);
			b3Quat rotation = b3Body_GetRotation(b3id);
			this->_pos.x = position.x;
			this->_pos.y = position.y;
			this->_pos.z = position.z;
			this->_rot.x = rotation.v.x;
			this->_rot.y = rotation.v.y;
			this->_rot.z = rotation.v.z;
			this->_rot.w = rotation.s;
		}
		updateMatrix();
	}
}

void Object::updateMatrix()
{
	this->_transform = MatrixMultiply(MatrixMultiply(MatrixScale(this->_scale.x, this->_scale.y, this->_scale.z), QuaternionToMatrix(this->_rot)), MatrixTranslate(this->_pos.x, this->_pos.y, this->_pos.z));

	if (this->_parent) {
		this->_transform = MatrixMultiply(this->_transform, this->_parent->_transform);
	}

}

void Object::draw(Playground* playground)
{
	playground->_models[this->_modelId].transform = this->_transform;
	playground->_models[this->_modelId].materials[0].maps[MATERIAL_MAP_ALBEDO].texture = playground->_textures[this->_texId];
	DrawModel(playground->_models[this->_modelId], Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
}



void Object::setParent(Object* obj)
{
	this->_parent = obj;
}