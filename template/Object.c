#include "Object.h"
#include "Playground.h"

#include <box3d/box3d.h>
#include <rlgl.h>


void ob_update(struct Object* self, Playground* playground)
{
	if (self->_type != OBJ_STATIC && self->_type != OBJ_OBSTACLE) {
		if (self->_physId != 0) {
			b3BodyId b3id = playground->_bodies[self->_physId];
			b3Vec3 position = b3Body_GetPosition(b3id);
			b3Quat rotation = b3Body_GetRotation(b3id);
			self->_pos.x = position.x;
			self->_pos.y = position.y;
			self->_pos.z = position.z;
			self->_rot.x = rotation.v.x;
			self->_rot.y = rotation.v.y;
			self->_rot.z = rotation.v.z;
			self->_rot.w = rotation.s;
		}
		self->updateMatrix(self);
	}
}

void ob_updateMatrix(struct Object* self)
{
	self->_transform = MatrixMultiply(MatrixMultiply(MatrixScale(self->_scale.x, self->_scale.y, self->_scale.z), QuaternionToMatrix(self->_rot)), MatrixTranslate(self->_pos.x, self->_pos.y, self->_pos.z));

	if (self->_parent) {
		self->_transform = MatrixMultiply(self->_transform, self->_parent->_transform);
	}

}

void ob_draw(struct Object* self, Playground* playground)
{
	playground->_models[self->_modelId].transform = self->_transform;
	playground->_models[self->_modelId].materials[0].maps[MATERIAL_MAP_ALBEDO].texture = playground->_textures[self->_texId];
	DrawModel(playground->_models[self->_modelId], (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
}



void ob_setParent(struct Object* self, struct Object* obj)
{
	self->_parent = obj;
}