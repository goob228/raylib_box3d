

#include "Camera.h"

#include <raylib.h>
#include <rcamera.h>


#include "Object.h"

#include "Playground.h"


Vector3 gc_rotatedPos(Object* self, Vector3 pos)
{
	CameraData* camdata = (CameraData*)self->data;
	float dist = 6.0f;
	Vector3 offset = {
		dist * cosf(camdata->_pitch + PI) * sinf(camdata->_yaw),
		dist * sinf(camdata->_pitch + PI),
		dist * cosf(camdata->_pitch + PI) * cosf(camdata->_yaw)
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
		camdata->_pitch -= playground->_my * camdata->_sensitivity;
		camdata->_pitch = Clamp(camdata->_pitch, -PI * 0.5f+0.01f, PI * 0.5f - 0.01f);
 		if (self->_parent) {
			camdata->_cam.position = camdata->rotatedPos(self, self->_pos);
			camdata->_cam.target = Vector3Transform(camdata->_target, self->_parent->_transform);
			
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


void gc_init(Object* self)
{
	self->_pos = (Vector3){ 0.0f, 2.0f, 0.0f };
	
	self->update = (&gc_update);
	self->setParent = (&ob_setParent);
	self->draw = (&gc_draw);

	CameraData* camdata = (CameraData*)self->data;

	camdata->init = (&gc_init);
	camdata->endFrame = (&gc_endFrame);
	camdata->startFrame = (&gc_startFrame);
	camdata->rotatedPos = (&gc_rotatedPos);

	camdata->_pitch = 0.0f;
	camdata->_yaw = 0.0f;
	camdata->_sensitivity = 0.001f;


	camdata->_target = (Vector3){ 0.0f, 2.0f, 0.0f };

	camdata->_cam.position = self->_pos;

	camdata->_cam.target = camdata->_target;
	camdata->_cam.up = camdata->_up;
	camdata->_cam.fovy = 90.0f;
	camdata->_cam.projection = CAMERA_PERSPECTIVE;

	camdata->_camMode = CAMERA_CUSTOM;
}