#include "MawJLaygo.h"
#include <assert.h>
#include "Stage.h"
#include "Player.h"

// デフォルトコンストラクタ
MawJLaygo::MawJLaygo() : MawJLaygo(VGet(0, 0, 0), 0.0f) {}

// コンストラクタ（初期化）
MawJLaygo::MawJLaygo(const VECTOR& pos, float rot)
{
	// モデルフォルダ
	const std::string folder = "data/model/Character/Maw j Laygo/";

	// モデル読み込み
	hModel = MV1LoadModel((folder + "Maw J Laygo.mv1").c_str());
	assert(hModel > 0);
	MV1SetupCollInfo(hModel);

	// モデルの向きを補正
	int root = MV1SearchFrame(hModel, "root");
	MV1SetFrameUserLocalMatrix(hModel, root, MGetRotY(DX_PI_F));

	// アニメーター作成
	animator = new Animator(hModel);
	assert(animator != nullptr);

	// アニメーション読み込み
	animator->AddFile(A_NEUTRAL, folder + "Mutant Idle.mv1", true);
	animator->AddFile(A_WALK, folder + "Walking.mv1", true);
	animator->AddFile(A_RUN, folder + "Run.mv1", true);
	animator->AddFile(A_ATTACK1, folder + "attack.mv1", false);
	animator->AddFile(A_DOWN, folder + "Receive Uppercut To The Facep.mv1", true);

	// 待機アニメ開始
	animator->Play(A_NEUTRAL);

	// 初期座標・向き
	transform.position = pos;
	transform.rotation.y = rot;
	transform.scale = VECTOR3(3, 3, 3);

	// 行動開始ステート
	state = ST_WAIT;
}

MawJLaygo::~MawJLaygo()
{
	if (animator != nullptr) {
		delete animator;
		animator = nullptr;
	}

	if (hModel > 0) {
		MV1DeleteModel(hModel);
		hModel = -1;
	}
}

void MawJLaygo::Update()
{

	animator->Update();

	// プレイヤー参照
	Player* player = FindGameObject<Player>();
	if (!player) return;

	// ステートごとの処理
	switch (state)
	{
	case ST_WAIT:
		UpdateWait();
		break;
	case ST_CHASE:
		UpdateChase();
		break;
	case ST_ATTACK:
		UpdateAttack();
		break;
	case ST_BACK:
		UpdateBack();
		break;
	case ST_DAMAGE:
		UpdateDamage();
		break;
	}

	// 地面に吸着させる
	Stage* stage = FindGameObject<Stage>();
	VECTOR hit;
	VECTOR pos1 = transform.position + VGet(0, 100, 0);
	VECTOR pos2 = transform.position + VGet(0, -200, 0);

	// 上から下にレイを飛ばして地面を取得
	if (stage->CollideLine(pos1, pos2, &hit))
	{
		transform.position = hit;
	}
}

// 描画
void MawJLaygo::Draw()
{
	Object3D::Draw();
}



void MawJLaygo::CheckAttack(VECTOR3 p1, VECTOR p2)
{
	MV1RefreshCollInfo(hModel);
	MV1_COLL_RESULT_POLY ret = MV1CollCheck_Line(hModel, -1, p1, p2);
	if (ret.HitFlag > 0) {
		animator->Play(A_DAMAGE);
		state = ST_DAMAGE;
	}
}


// ▼ ST_WAIT：待機（プレイヤーを探す）
void MawJLaygo::UpdateWait()
{
	Player* pl = FindGameObject<Player>();
	VECTOR3 plPos = pl->GetTransform().position;//playerの座標
	VECTOR3 toPlayer = plPos - transform.position;
	// toPlayerの長さが3000以下か？
	if (toPlayer.Size() > 4000)
		return;

	VECTOR3 front = VECTOR3(0, 0, -1) * MGetRotY(transform.rotation.y);
	// toPlayerの長さ１にしたのと、frontで内積をとる
	float cosT = VDot(toPlayer.Normalize(), front);
	// θが60°以内か？
	if (cosT >= cos(60 * DegToRad)) {
		animator->Play(A_RUN);
		state = ST_CHASE; // 仮
	}

}

// ST_CHASE：追跡（プレイヤーに近づく）
void MawJLaygo::UpdateChase()
{

	Player* pl = FindGameObject<Player>();//プレイヤーのオブジェクトを取得する。

	if (!pl) return;//player が存在しない（null の）場合は、この関数をすぐに終了する

	float d = MoveTo(pl->GetTransform().position, 6);;// プレイヤーに向かって移動 速度

	animator->Play(A_WALK);

	// 近づいたらATTACKへ
	if (d < 100.0f) {
		animator->Play(A_ATTACK1);
		state = ST_ATTACK;
	}
	// テリトリーを出たらWAITにする
	VECTOR3 v = transform.position - territory.center;
	if (v.Size() >= territory.range) {
		animator->Play(A_WALK);
		state = ST_BACK;
	}
}

// ▼ ST_ATTACK：攻撃
void MawJLaygo::UpdateAttack()
{
	// 攻撃アニメーションが終わったらWAIT
	if (animator->IsFinish()) {
		animator->Play(A_NEUTRAL);
		state = ST_WAIT;
	}
	float frame = animator->GetCurrentFrame();
	if (frame < 20.0f) {
		//Playerの方を向く
		Player* pl = FindGameObject<Player>();
		VECTOR3 toPl = pl->GetTransform().position - transform.position;
		//		transform.rotation.y = atan2(toPl.x, toPl.z);
		VECTOR3 right = VECTOR3(1, 0, 0) * transform.GetRotationMatrix();
		if (VDot(right, toPl) > 0) {
			transform.rotation.y += 2.0f * DegToRad;
		}
		else {
			transform.rotation.y -= 2.0f * DegToRad;
		}
	}
	if (frame >= 24.5 && frame <= 27.5) {
		Player* pl = FindGameObject<Player>();

	}
	/*if (animator->IsFinish())
	{
		animator->Play(A_NEUTRAL);
		state = ST_WAIT;
	}
	float frame = animator->GetCurrentFrame();
	if (frame < 20.0f)
	{
		Player* pl = FindGameObject<Player>();
		VECTOR3 toPl = pl->GetTransform().position - transform.position;
		toPl.y = 0;

		VECTOR3 front = VECTOR3(0, 0, 1) * MGetRotY(transform.rotation.y);

		// プレイヤーが「右にいる」か「左にいる」か
		VECTOR3 right = VECTOR3(1, 0, 0) * MGetRotY(transform.rotation.y);
		float ip = VDot(right, toPl);

		float turnSpeed = 1.5f * DegToRad;

		if (ip > 0)
			transform.rotation.y += turnSpeed;
		else
			transform.rotation.y -= turnSpeed;
	}*/
	
}

// ▼ ST_BACK：後退（少し離れる）
void MawJLaygo::UpdateBack()
{
	float d = MoveTo(territory.center, 2);
	if (d < 100.0f) {
		animator->Play(A_NEUTRAL);
		state = ST_WAIT;
	}
}

// ▼ ST_DAMAGE：ダメージ中
void MawJLaygo::UpdateDamage()
{

}

/*---------------------------------------------
	移動関数：MoveTo
	指定座標に向かって speed で移動する
----------------------------------------------*/
// ------------------------------------------------------------
// プレイヤーへ向かって移動しつつ、正しい方向へ回転する処理
// return：ターゲットとの距離
// ------------------------------------------------------------
float MawJLaygo::MoveTo(VECTOR3 target, float speed)
{
	/*
	// ----------------------------
	// ① 今の向き（Y回転）を使って前方向へ進む
	// ----------------------------
	// forward = (0,0,1) を現在のY回転で回したもの
	VECTOR3 forward = VECTOR3(0, 0, 1) * MGetRotY(transform.rotation.y);

	// 前に進む速度ベクトル
	VECTOR3 velocity = forward * speed;

	// プレイヤーへ移動
	transform.position += velocity;


	// ----------------------------
	// ② プレイヤー方向へ回転する処理
	// ----------------------------
	// 敵 → プレイヤー の方向ベクトル
	VECTOR3 toTarget = target - transform.position;

	// 今の「右方向」＝(1,0,0) を Y回転したもの
	VECTOR3 right = VECTOR3(1, 0, 0) * MGetRotY(transform.rotation.y);

	// 内積 → 正なら右側、負なら左側にプレイヤーがいる
	float ip = VDot(right, toTarget);

	// プレイヤーが右にいる → 時計回りに回転
	if (ip > 0)
	{
		transform.rotation.y += 1.0f * DegToRad;   // ←回しすぎないよう小さめ
	}
	// プレイヤーが左にいる → 反時計回り
	else
	{
		transform.rotation.y -= 1.0f * DegToRad;
	}


	// ----------------------------
	// ③ ターゲットまでの距離を返す
	// （DXLib には Size() が無い → VSize を使う）
	// ----------------------------
	return VSize(toTarget);*/

	VECTOR3 velocity = VECTOR3(0, 0, speed) * MGetRotY(transform.rotation.y);
	transform.position += velocity;
	VECTOR3 right = VECTOR3(1, 0, 0) * MGetRotY(transform.rotation.y);
	VECTOR3 toTarget = target - transform.position;
	float ip = VDot(right, toTarget);
	if (ip >= 0)
		transform.rotation.y += DegToRad;
	else
		transform.rotation.y -= DegToRad;
	return toTarget.Size();
}
