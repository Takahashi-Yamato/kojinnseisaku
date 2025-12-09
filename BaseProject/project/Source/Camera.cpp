#include "Camera.h"

namespace {
	static float distance = 500.0f; // キャラからの距離
	static float lookHeight = 200.0f; // 注視点の高さ
};


Camera::Camera()
{
	GetMousePoint(&prevMouseX, &prevMouseY);
}


Camera::~Camera()
{
}

void Camera::Update()
{
	int mouseX, mouseY;
	GetMousePoint(&mouseX, &mouseY); // マウスの位置が入る
	int moveX = mouseX - prevMouseX; // マウス移動量
	int moveY = mouseY - prevMouseY;
	// カメラ回転操作
	transform.rotation.y += moveX * 0.3f * DegToRad;
	transform.rotation.x += moveY * 0.1f * DegToRad;
	prevMouseX = mouseX;
	prevMouseY = mouseY;

	if (transform.rotation.x >= 85.0f * DegToRad) {
		transform.rotation.x = 85.0f * DegToRad;
	}
	if (transform.rotation.x <= -45.0f * DegToRad) {
		transform.rotation.x = -45.0f * DegToRad;
	}

	VECTOR3 camPos = lookPosition + VECTOR3(0, 150, 0)
		+ VECTOR3(0, 0, -427) * MGetRotX(transform.rotation.x)
		* MGetRotY(transform.rotation.y);
	VECTOR3 lookPos = lookPosition + VECTOR3(0, 200, 0);
	SetCameraPositionAndTarget_UpVecY(camPos, lookPos);
}

void Camera::Draw()
{
}

void Camera::SetPlayerPosition(const VECTOR& pos)
{
	lookPosition = pos;
}
