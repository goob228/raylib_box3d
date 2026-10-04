

#include "Camera.h"

#include <raylib.h>
#include <rcamera.h>


#include "Object.h"

#include "G_local.h"
#include "MainModule.h"

extern EventHandler eventhandlers[5];


extern Object camera;

Vector3 gc_rotatedPos(Object* self, Vector3 pos)
{
	CameraData* camdata = (CameraData*)self->data;
	Vector3 offset = {
		camdata->dist * cosf(camdata->pitch + PI) * sinf(camdata->yaw),
		camdata->dist * sinf(camdata->pitch + PI),
		camdata->dist * cosf(camdata->pitch + PI) * cosf(camdata->yaw)
	};

	Vector3 outp = pos;

	if (self->parent) {
		ecs_entity_t e = ecs_lookup(g_world, "Player");
		Drawable* drawable = ecs_get_mut(g_world, e, Drawable);
		outp = Vector3Transform(outp, drawable->transform);
	}
	
	return Vector3Add(outp, offset);
}

#define MYLERP(a, b, x) (a + x*(b-a))
#define MYCLAMP(x) (x > 1 ? 1 : (x > 0 ? x : 0))

void gc_update(struct Object* obj)
{
	
	Object* self = obj;

	CameraData* camdata = (CameraData*)self->data;

	if (camdata->camMode == CAMERA_CUSTOM) {
		camdata->wanna_yaw -= eventhandlers[0].mx * camdata->sensitivity;
		camdata->wanna_pitch -= eventhandlers[0].my * camdata->sensitivity * (camdata->type == CAM_FIRST_PERSON ? -1.0f : 1.0f);
		camdata->wanna_pitch = Clamp(camdata->wanna_pitch, -PI * 0.5f+0.01f, PI * 0.5f - 0.01f);
		double dt = (double)GetFrameTime() * 25.0;
		dt = (double)MYCLAMP(dt);
		camdata->yaw = MYLERP(camdata->yaw, camdata->wanna_yaw, dt);
		camdata->pitch = MYLERP(camdata->pitch, camdata->wanna_pitch, dt);
		camdata->dist *= (1.0f - eventhandlers[0].s*0.1f);
 		if (self->parent) {
			ecs_entity_t e = ecs_lookup(g_world, "Player");
			Position* pos = ecs_get_mut(g_world, e, Position);
			Rotater* rot = ecs_get_mut(g_world, e, Rotater);
			Drawable* drawable = ecs_get_mut(g_world, e, Drawable);
			//ob_updateMatrix2(self->parent, pos, rot);
			if (camdata->type == CAM_FIRST_PERSON) {
				camdata->cam.target = camdata->rotatedPos(self, camdata->target);
				camdata->cam.position = Vector3Transform(self->pos, drawable->transform);
			} else {
				camdata->cam.position = camdata->rotatedPos(self, self->pos);
				camdata->cam.target = Vector3Transform(camdata->target, drawable->transform);
			}
			Vector3 right = Vector3CrossProduct(Vector3Subtract(camdata->cam.target, camdata->cam.position), (Vector3){ 0.0f,-1.0f, 0.0f });
			Vector3 newup = Vector3CrossProduct(Vector3Subtract(camdata->cam.target, camdata->cam.position), right);
			newup = Vector3Normalize(newup);
			camdata->cam.up = newup;
			//cam.up = _up;
		}
		else {
			camdata->cam.position = camdata->rotatedPos(self, self->pos);
			camdata->cam.target = camdata->target;
			camdata->cam.up = camdata->up;
		}
	}

	



	UpdateCamera(&camdata->cam, camdata->camMode);
}

void gc_startFrame(Object* self)
{
	CameraData* camdata = (CameraData*)self->data;
	BeginMode3D(camdata->cam);
}

void gc_endFrame(Object* self)
{
	EndMode3D();
}

void gc_draw(struct Object* self, Playground* playground)
{

}

b3Vec3 gc_getForward(Object* self)
{
	CameraData* cd = (CameraData*)self->data;
	return (b3Vec3){cd->cam.target.x - cd->cam.position.x, cd->cam.target.y - cd->cam.position.y, cd->cam.target.z - cd->cam.position.z};
}

b3Vec3 gc_getRight(Object* self)
{
	CameraData* cd = (CameraData*)self->data;
	Vector3 right = Vector3CrossProduct(Vector3Subtract(cd->cam.target, cd->cam.position), (Vector3){ 0.0f,-1.0f, 0.0f });
	return (b3Vec3){right.x, right.y, right.z};
}


void gc_init(Object* self)
{
	self->pos = (Vector3){ 0.0f, 0.25f, 0.0f };
	
	self->update = (&gc_update);
	self->setParent = (&ob_setParent);

	CameraData* camdata = (CameraData*)self->data;

	camdata->init = (&gc_init);
	camdata->endFrame = (&gc_endFrame);
	camdata->startFrame = (&gc_startFrame);
	camdata->getForward = (&gc_getForward);
	camdata->getRight = (&gc_getRight);
	camdata->rotatedPos = (&gc_rotatedPos);

	camdata->pitch = 0.0f;
	camdata->yaw = 0.0f;
	camdata->sensitivity = 0.001f;
	camdata->dist = 1.0f;


	camdata->target = (Vector3){ 0.0f, 0.25f, 0.0f };

	camdata->cam.position = self->pos;

	camdata->cam.target = camdata->target;
	camdata->cam.up = camdata->up;
	camdata->cam.fovy = 110.0f;
	camdata->cam.projection = CAMERA_PERSPECTIVE;

	camdata->camMode = CAMERA_CUSTOM;
	camdata->type = CAM_FIRST_PERSON;
}