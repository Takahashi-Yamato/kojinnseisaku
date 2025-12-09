#pragma once
#include "Object3D.h"

class StageObject : public Object3D {
public:
	StageObject(const std::string& fileName, const VECTOR3& position, const VECTOR3& rotation, const VECTOR3& scale);
	~StageObject();
	void Draw() override;
	void PostDraw();
	int HitModel() { return hHitModel; }
	bool CollideLine(const VECTOR& pos1, const VECTOR& pos2, VECTOR* hit = nullptr) const;

private:
	int hHitModel;
};