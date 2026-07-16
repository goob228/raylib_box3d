

#include "Camera.h"

#include <raylib.h>
#include <rcamera.h>


#include "Object.h"

#include "Playground.h"

Vector3 GameCamera::rotatedPos(Vector3 pos)
{
	float dist = 6.0f;
	Vector3 offset = {
		dist * cosf(this->_pitch + PI) * sinf(this->_yaw),
		dist * sinf(this->_pitch + PI),
		dist * cosf(this->_pitch + PI) * cosf(this->_yaw)
	};

	Vector3 outp = pos;

	if (this->_parent) {
		outp = Vector3Transform(outp, this->_parent->_transform);
	}
	
	return outp + offset;
}

void GameCamera::init()
{
	
	this->_pos = Vector3{ 0.0f, 2.0f, 0.0f };
	this->_target = Vector3{ 0.0f, 2.0f, 0.0f };

	this->_cam.position = this->_pos;

	this->_cam.target = this->_target;
	this->_cam.up = this->_up;
	this->_cam.fovy = 90.0f;
	this->_cam.projection = CAMERA_PERSPECTIVE;
	
	this->_camMode = CAMERA_CUSTOM;

}

void GameCamera::update(Playground* playground)
{
	
	if (this->_camMode == CAMERA_CUSTOM) {
		this->_yaw -= playground->_mx * this->_sensitivity;
		this->_pitch -= playground->_my * this->_sensitivity;
		this->_pitch = Clamp(this->_pitch, -PI * 0.5f+0.01f, PI * 0.5f - 0.01f);
		if (this->_parent) {
			this->_cam.position = rotatedPos(this->_pos);
			this->_cam.target = Vector3Transform(this->_target, this->_parent->_transform);
			Vector3 right = Vector3CrossProduct(this->_cam.target - this->_cam.position, Vector3{ 0.0f,-1.0f, 0.0f });
			Vector3 newup = Vector3CrossProduct(this->_cam.target - this->_cam.position, right);
			newup = Vector3Normalize(newup);
			this->_cam.up = newup;
			//_cam.up = _up;
		}
		else {
			this->_cam.position = rotatedPos(this->_pos);
			this->_cam.target = this->_target;
			this->_cam.up = this->_up;
		}
	}

	



	UpdateCamera(&this->_cam, this->_camMode);
}

void GameCamera::startFrame()
{
	BeginMode3D(this->_cam);
}

void GameCamera::endFrame()
{
	EndMode3D();
}

void GameCamera::draw(Playground* playground)
{

}
