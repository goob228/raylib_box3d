#include "Object.h"
#include "Playground.h"

#include <box3d/box3d.h>
#include <rlgl.h>


#include "Resource.h"

void ob_update(struct Object* self, Playground* playground)
{
	if (self->type != OBJ_STATIC && self->type != OBJ_OBSTACLE) {
		if (self->physId != 0) {
			b3BodyId b3id = playground->bodies[self->physId];
			b3Vec3 position = b3Body_GetPosition(b3id);
			b3Quat rotation = b3Body_GetRotation(b3id);
			self->pos.x = position.x;
			self->pos.y = position.y;
			self->pos.z = position.z;
			self->rot.x = rotation.v.x;
			self->rot.y = rotation.v.y;
			self->rot.z = rotation.v.z;
			self->rot.w = rotation.s;
		}
		self->updateMatrix(self);
	}
}

void ob_updateMatrix(struct Object* self)
{
	self->transform = MatrixMultiply(MatrixMultiply(MatrixScale(self->scale.x, self->scale.y, self->scale.z), QuaternionToMatrix(self->rot)), MatrixTranslate(self->pos.x, self->pos.y, self->pos.z));

	if (self->parent) {
		self->transform = MatrixMultiply(self->transform, self->parent->transform);
	}

}

void ob_draw(struct Object* self, Playground* playground)
{	
	Model md = getModelResource(&(self->modelres));
	md.transform = self->transform;
	if (self->texres.id)
		md.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = getTextureResource(&(self->texres));

	
	DrawModel(md, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
}



void ob_setParent(struct Object* self, struct Object* obj)
{
	self->parent = obj;
}