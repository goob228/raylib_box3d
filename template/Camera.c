

#include "Camera.h"

#include <raylib.h>
#include <rcamera.h>


#include "Object.h"

#include "Playground.h"


Vector3 gc_rotatedPos(struct GameCamera* self, Vector3 pos)
{
	float dist = 6.0f;
	Vector3 offset = {
		dist * cosf(self->_pitch + PI) * sinf(self->_yaw),
		dist * sinf(self->_pitch + PI),
		dist * cosf(self->_pitch + PI) * cosf(self->_yaw)
	};

	Vector3 outp = pos;

	if (self->_parent) {
		outp = Vector3Transform(outp, self->_parent->_transform);
	}
	
	return Vector3Add(outp, offset);
}



void gc_update(struct Object* obj, Playground* playground)
{
	
	struct GameCamera* self = (struct GameCamera*)obj;

	if (self->_camMode == CAMERA_CUSTOM) {
		self->_yaw -= playground->_mx * self->_sensitivity;
		self->_pitch -= playground->_my * self->_sensitivity;
		self->_pitch = Clamp(self->_pitch, -PI * 0.5f+0.01f, PI * 0.5f - 0.01f);
 		if (self->_parent) {
			self->_cam.position = self->rotatedPos(self, self->_pos);
			self->_cam.target = Vector3Transform(self->_target, self->_parent->_transform);
			
			Vector3 right = Vector3CrossProduct(Vector3Subtract(self->_cam.target, self->_cam.position), (Vector3){ 0.0f,-1.0f, 0.0f });
			Vector3 newup = Vector3CrossProduct(Vector3Subtract(self->_cam.target, self->_cam.position), right);
			newup = Vector3Normalize(newup);
			self->_cam.up = newup;
			//_cam.up = _up;
		}
		else {
			self->_cam.position = self->rotatedPos(self, self->_pos);
			self->_cam.target = self->_target;
			self->_cam.up = self->_up;
		}
	}

	



	UpdateCamera(&self->_cam, self->_camMode);
}

void gc_startFrame(struct GameCamera* self)
{
	BeginMode3D(self->_cam);
}

void gc_endFrame(struct GameCamera* self)
{
	EndMode3D();
}

void gc_draw(struct Object* self, Playground* playground)
{

}


void gc_init(struct GameCamera* self)
{

	self->init = (&gc_init);
	self->endFrame = (&gc_endFrame);
	self->startFrame = (&gc_startFrame);
	self->update = (&gc_update);
	self->rotatedPos = (&gc_rotatedPos);
	self->setParent = (&ob_setParent);
	self->draw = (&gc_draw);

	self->_pitch = 0.0f;
	self->_yaw = 0.0f;
	self->_sensitivity = 0.001f;

	self->_pos = (Vector3){ 0.0f, 2.0f, 0.0f };
	self->_target = (Vector3){ 0.0f, 2.0f, 0.0f };

	self->_cam.position = self->_pos;

	self->_cam.target = self->_target;
	self->_cam.up = self->_up;
	self->_cam.fovy = 90.0f;
	self->_cam.projection = CAMERA_PERSPECTIVE;

	self->_camMode = CAMERA_CUSTOM;
}