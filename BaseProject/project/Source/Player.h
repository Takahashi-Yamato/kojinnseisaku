#include "Object3D.h"
#include "Animator.h"
#include "StateBase.h"

class Camera;

class Player : public Object3D
{
public:
	Player();
	Player(const VECTOR3& pos, float rot);
	~Player();

	void Update() override;  // 毎フレーム呼ばれる更新処理
	void Draw() override;    // キャラ描画

private:
	// ---- アニメーションID ----
	// Animatorに登録された各アニメーションを識別するためのID
	enum AnimID {
		A_NEUTRAL = 0,   // 待機
		A_RUN,            // 走り
		A_BACKRUN,        // 後退
		A_ATTACK1,        // 通常攻撃
		A_ATTACK2,
		A_ATTACK3,
		A_JUMP,           // ジャンプ
		A_JUMPATTACK1,    // ジャンプ攻撃
	};

	// ---- プレイヤーの状態管理 ----
	// 現在どんな行動をしているかを表す
	enum State {
		ST_NORMAL = 0,     // 通常状態（地上）
		ST_ATTACK1,        // 攻撃中
		ST_ATTACK2,
		ST_ATTACK3,
		ST_JUMP,           // ジャンプ中
		ST_JUMPATTACK1,    // 空中攻撃中
	};
	State state; // 現在の状態を保持する変数

	Animator* animator;
	Camera* camera;

	bool Ispush; // ボタンを押したかどうか

	// 各状態ごとの更新処理
	void UpdateNormal();

	void UpdateJump();

	void UpdateRotationFromMouse();

	void UpdateAttack();

	void UpdateAttack1();

	void UpdateAttack2();

	void UpdateAttack3();

	void UpdateJumpAttack();

	bool attackNext;
	
	


private:
	// ---- ジャンプ物理関連 ----
	float velocityY;   // 上下方向の速度（上昇・落下を表す）
	float gravity;     // 重力加速度
	bool isGrounded;   // 地面にいるかどうか（trueなら着地状態）
	VECTOR3 GetForward() const;

};
