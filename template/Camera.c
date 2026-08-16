

#include "Camera.h"

#include <raylib.h>
#include <rcamera.h>


#include "Object.h"

#include "Playground.h"


Vector3 gc_rotatedPos(Object* self, Vector3 pos)
{
	CameraData* camdata = (CameraData*)self->data;
	Vector3 offset = {
		camdata->dist * cosf(camdata->_pitch + PI) * sinf(camdata->_yaw),
		camdata->dist * sinf(camdata->_pitch + PI),
		camdata->dist * cosf(camdata->_pitch + PI) * cosf(camdata->_yaw)
	};

	Vector3 outp = pos;

	if (self->_parent) {
		outp = Vector3Transform(outp, self->_parent->_transform);
	}
	
	return Vector3Add(outp, offset);
}



void gc_update(struct Object* obj, Playground* playground)
{
	
	Object* self = obj;

	CameraData* camdata = (CameraData*)self->data;

	if (camdata->_camMode == CAMERA_CUSTOM) {
		camdata->_yaw -= playground->_mx * camdata->_sensitivity;
		camdata->_pitch -= playground->_my * camdata->_sensitivity * (camdata->type == CAM_FIRST_PERSON ? -1.0f : 1.0f);
		camdata->_pitch = Clamp(camdata->_pitch, -PI * 0.5f+0.01f, PI * 0.5f - 0.01f);
		camdata->dist *= (1.0f - GetMouseWheelMove()*0.1f);
 		if (self->_parent) {
			self->_parent->updateMatrix(self->_parent);
			if (camdata->type == CAM_FIRST_PERSON) {
				camdata->_cam.target = camdata->rotatedPos(self, camdata->_target);
				camdata->_cam.position = Vector3Transform(self->_pos, self->_parent->_transform);
			} else {
				camdata->_cam.position = camdata->rotatedPos(self, self->_pos);
				camdata->_cam.target = Vector3Transform(camdata->_target, self->_parent->_transform);
			}
			Vector3 right = Vector3CrossProduct(Vector3Subtract(camdata->_cam.target, camdata->_cam.position), (Vector3){ 0.0f,-1.0f, 0.0f });
			Vector3 newup = Vector3CrossProduct(Vector3Subtract(camdata->_cam.target, camdata->_cam.position), right);
			newup = Vector3Normalize(newup);
			camdata->_cam.up = newup;
			//_cam.up = _up;
		}
		else {
			camdata->_cam.position = camdata->rotatedPos(self, self->_pos);
			camdata->_cam.target = camdata->_target;
			camdata->_cam.up = camdata->_up;
		}
	}

	



	UpdateCamera(&camdata->_cam, camdata->_camMode);
}

void gc_startFrame(Object* self)
{
	CameraData* camdata = (CameraData*)self->data;
	BeginMode3D(camdata->_cam);
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
	return (b3Vec3){cd->_cam.target.x - cd->_cam.position.x, cd->_cam.target.y - cd->_cam.position.y, cd->_cam.target.z - cd->_cam.position.z};
}

b3Vec3 gc_getRight(Object* self)
{
	CameraData* cd = (CameraData*)self->data;
	Vector3 right = Vector3CrossProduct(Vector3Subtract(cd->_cam.target, cd->_cam.position), (Vector3){ 0.0f,-1.0f, 0.0f });
	return (b3Vec3){right.x, right.y, right.z};
}


void gc_init(Object* self)
{
	self->_pos = (Vector3){ 0.0f, 1.5f, 0.0f };
	
	self->update = (&gc_update);
	self->setParent = (&ob_setParent);
	self->draw = (&gc_draw);

	CameraData* camdata = (CameraData*)self->data;

	camdata->init = (&gc_init);
	camdata->endFrame = (&gc_endFrame);
	camdata->startFrame = (&gc_startFrame);
	camdata->getForward = (&gc_getForward);
	camdata->getRight = (&gc_getRight);
	camdata->rotatedPos = (&gc_rotatedPos);

	camdata->_pitch = 0.0f;
	camdata->_yaw = 0.0f;
	camdata->_sensitivity = 0.001f;
	camdata->dist = 1.0f;


	camdata->_target = (Vector3){ 0.0f, 1.5f, 0.0f };

	camdata->_cam.position = self->_pos;

	camdata->_cam.target = camdata->_target;
	camdata->_cam.up = camdata->_up;
	camdata->_cam.fovy = 90.0f;
	camdata->_cam.projection = CAMERA_PERSPECTIVE;

	camdata->_camMode = CAMERA_CUSTOM;
	camdata->type = CAM_FIRST_PERSON;
}