#pragma once
#include "Object3D.h"
#include "../Library/myDxLib.h"


class Camera : public Object3D {
public:
	Camera();
	~Camera();
	void Update() override;
	void Draw() override;

	void SetPlayerPosition(const VECTOR& pos);
private:
	VECTOR lookPosition;
	int prevMouseX, prevMouseY;
};