#include "Object.h"
#include "Playground.h"

#include <box3d/box3d.h>
#include <rlgl.h>


#include "Resource.h"
#include "G_local.h"

void ob_update(struct Object* self, Playground* playground)
{
	if (self->type != OBJ_STATIC && self->type != OBJ_OBSTACLE) {
		if (self->physId != 0) {
			b3BodyId b3id = playground->bodies[self->physId];

			self->posPrev = self->posCurr;
			self->rotPrev = self->rotCurr;

			self->posCurr = b3Body_GetPosition(b3id);
			self->rotCurr = b3Body_GetRotation(b3id);
		}
		
	}
}

void ob_updateMatrix(struct Object* self)
{

	b3Pos lerped = b3LerpPosition(self->posPrev, self->posCurr, (float)alphaBlend);
	Quaternion lerpedRot = QuaternionNlerp(
		(Quaternion){self->rotPrev.v.x,self->rotPrev.v.y,self->rotPrev.v.z,self->rotPrev.s },
		(Quaternion){self->rotCurr.v.x,self->rotCurr.v.y,self->rotCurr.v.z,self->rotCurr.s },
		(float)alphaBlend	);
	self->pos.x = lerped.x;
	self->pos.y = lerped.y;
	self->pos.z = lerped.z;

	self->rot = lerpedRot;

	self->transform = MatrixMultiply(MatrixMultiply(MatrixScale(self->scale.x, self->scale.y, self->scale.z), QuaternionToMatrix(self->rot)), MatrixTranslate(self->pos.x, self->pos.y, self->pos.z));

	if (self->parent) {
		self->parent->updateMatrix(self->parent);
		self->transform = MatrixMultiply(self->transform, self->parent->transform);
	}

}

void ob_draw(struct Object* self, Playground* playground)
{	
	self->updateMatrix(self);
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