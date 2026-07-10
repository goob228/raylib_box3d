#include "Camera.h"

#include <raylib.h>
#include <rcamera.h>

#include "Object.h"
#include "Playground.h"


Vector3 GameCamera::rotatedPos(Vector3 pos)
{
	float dist = 6.0f;
	Vector3 offset = {
		dist * cosf(_pitch) * sinf(_yaw),
		dist * sinf(_pitch),
		dist * cosf(_pitch) * cosf(_yaw)
	};

	Vector3 outp = pos;

	if (_parent) {
		outp = Vector3Transform(outp, _parent->_transform);
	}
	
	return outp + offset;
}

void GameCamera::init()
{
	
	_pos = Vector3{ 0.0f, 2.0f, 0.0f };
	_target = Vector3{ 0.0f, 2.0f, 0.0f };

	_cam.position = _pos;

	_cam.target = _target;
	_cam.up = _up;
	_cam.fovy = 90.0f;
	_cam.projection = CAMERA_PERSPECTIVE;
	
	_camMode = CAMERA_CUSTOM;

}

void GameCamera::update(Playground* playground)
{
	
	if (_camMode == CAMERA_CUSTOM) {
		_yaw -= playground->_mx * _sensitivity;
		_pitch += playground->_my * _sensitivity;
		_pitch = Clamp(_pitch, -PI * 0.5f+0.01f, PI * 0.5f - 0.01f);
		if (_parent) {
			_cam.position = rotatedPos(_pos);
			_cam.target = Vector3Transform(_target, _parent->_transform);
			//_cam.up = Vector3Transform(_up, _parent->_transform);
			_cam.up = _up;
		}
		else {
			_cam.position = rotatedPos(_pos);
			_cam.target = _target;
			_cam.up = _up;
		}
	}


	UpdateCamera(&_cam, _camMode);
}

void GameCamera::startFrame()
{
	BeginMode3D(_cam);
}

void GameCamera::endFrame()
{
	EndMode3D();
}

void GameCamera::draw(Playground* playground)
{

}
