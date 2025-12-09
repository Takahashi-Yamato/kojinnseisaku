#pragma once
#include "Object3D.h"

class Enemy : public Object3D {
public:
	virtual void CheckAttack(VECTOR3 p1, VECTOR3 p2) {}
};