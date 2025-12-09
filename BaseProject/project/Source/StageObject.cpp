#include "StageObject.h"
#include <assert.h>

StageObject::StageObject(const std::string& fileName, const VECTOR3& position, const VECTOR3& rotation, const VECTOR3& scale)
{
	const std::string folder = "data/model/Stage/";
	hModel = MV1LoadModel((folder + fileName + ".mv1").c_str());
	assert(hModel > 0);
	transform.position = position;
	transform.rotation = rotation;
	transform.scale = scale;
	transform.MakeLocalMatrix();
	MV1SetMatrix(hModel, transform.GetLocalMatrix());

	hHitModel = MV1LoadModel((folder + fileName + "_c.mv1").c_str());
	if (hHitModel > 0) {
		MV1SetMatrix(hHitModel, transform.GetLocalMatrix());
		MV1SetupCollInfo(hHitModel);
	}
}

StageObject::~StageObject()
{
	if (hHitModel > 0) {
		MV1DeleteModel(hHitModel);
		hHitModel = -1;
	}
	if (hModel > 0) {
		MV1DeleteModel(hModel);
		hModel = -1;
	}
}


void StageObject::Draw()
{
	if (IsTag("Stage_Obj001")) {
		MV1SetSemiTransDrawMode(DX_SEMITRANSDRAWMODE_NOT_SEMITRANS_ONLY);
		Object3D::Draw();
		MV1SetSemiTransDrawMode(DX_SEMITRANSDRAWMODE_ALWAYS);
	}
	else {
		Object3D::Draw();
	}
}


void StageObject::PostDraw()
{
	if (IsTag("Stage_Obj001")) {
		MV1SetSemiTransDrawMode(DX_SEMITRANSDRAWMODE_SEMITRANS_ONLY);
		//		MV1SetUseZBuffer(hModel, FALSE);
		Object3D::Draw();
		MV1SetUseZBuffer(hModel, TRUE);
		MV1SetSemiTransDrawMode(DX_SEMITRANSDRAWMODE_ALWAYS);
	}
}

bool StageObject::CollideLine(const VECTOR& pos1, const VECTOR& pos2, VECTOR* hit) const
{
	MV1_COLL_RESULT_POLY ret = MV1CollCheck_Line(hModel, -1, pos1, pos2);
	if (ret.HitFlag > 0) {
		*hit = ret.HitPosition;
		return true;
	}
	return false;
}
