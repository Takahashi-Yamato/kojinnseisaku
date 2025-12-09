#include "Player.h"
#include <assert.h>
#include "Camera.h"
#include "Stage.h"
#include "DxLib.h"

// -----------------------------------------
// キーが「押した瞬間」だけ true になる関数
// oldKey に前フレームの状態を記録して比較
// -----------------------------------------
bool IsKeyPressed(int key)
{
    static int oldKey[256] = { 0 };  // 前フレームのキー状態保持
    int now = CheckHitKey(key);      // 現在のキー状態

    bool pressed = (now && !oldKey[key]); // 今押されていて前フレーム押されていない → 今押した瞬間
    oldKey[key] = now;                       // 状態更新

    return pressed;
}

// -----------------------------------------
// マウスが「クリックした瞬間」だけ true になる関数
// -----------------------------------------
bool IsMouseClicked(int button)
{
    static int oldState[3] = { false, false, false }; // 前フレームのマウスボタン状態

    int now = (GetMouseInput() & (1 << button)) != 0; // 今押しているか
    bool clicked = (now && !oldState[button]);         // 押した瞬間？

    oldState[button] = now;                             // 状態更新
    return clicked;
}

// -----------------------------------------
// デフォルトコンストラクタ（位置0 回転0）
// -----------------------------------------
Player::Player() : Player(VGet(0, 0, 0), 0.0f) {}

// -----------------------------------------
// プレイヤー生成
// -----------------------------------------
Player::Player(const VECTOR3& pos, float rot) : Ispush(false)
{
    const std::string folder = "data/model/Character/Player/";

    // ▼ プレイヤーモデルの読み込み
    hModel = MV1LoadModel((folder + "Paladin WProp J Nordstrom.mv1").c_str());
    assert(hModel > 0);

    // ▼ root フレームを回転（初期向き調整）
    int root = MV1SearchFrame(hModel, "root");
    MV1SetFrameUserLocalMatrix(hModel, root, MGetRotY(DX_PI_F));

    // ▼ アニメーター生成
    animator = new Animator(hModel);
    assert(animator != nullptr);

    // ▼ アニメーション登録（true=ループ再生）
    animator->AddFile(A_NEUTRAL, folder + "Sword And Shield Idle.mv1", true);
    animator->AddFile(A_RUN, folder + "Sword And Shield Run.mv1", true, 1.0f, 1);
    animator->AddFile(A_BACKRUN, folder + "BackWalk.mv1", true, 1.0f, 1);

    // 攻撃アニメはループさせないので false
    animator->AddFile(A_ATTACK1, folder + "Sword And Shield Slash 1.mv1", false, 2.0f);
    animator->AddFile(A_ATTACK2, folder + "Sword And Shield Slash 2.mv1", false, 2.0f);
    animator->AddFile(A_ATTACK3, folder + "Sword And Shield Slash3.mv1", false, 2.0f);

    animator->AddFile(A_JUMP, folder + "Sword And Shield Jump.mv1", true, 1.0f, 1);
    animator->AddFile(A_JUMPATTACK1, folder + "JumpAttack.mv1", false, 1.8f);

    animator->Play(A_NEUTRAL);  // 最初は待機アニメ

    // 位置・回転セット
    transform.position = pos;
    transform.rotation.y = rot;

    // カメラ取得（ゲーム内の Camera オブジェクト）
    camera = FindGameObject<Camera>();

    // 初期状態は通常
    state = ST_NORMAL;

    // ▼ ジャンプ関連初期化
    velocityY = 0.0f;   // Y速度
    gravity = -0.5f;    // 重力
    isGrounded = true;  // 地面にいる状態
}

// -----------------------------------------
// デストラクタ（モデルとアニメーターの後片付け）
// -----------------------------------------
Player::~Player()
{
    if (hModel > 0)
    {
        MV1DeleteModel(hModel);
        hModel = -1;
    }
    if (animator != nullptr)
    {
        delete animator;
        animator = nullptr;
    }
}

VECTOR3 moveVec; // プレイヤーが移動しようとしている方向ベクトル

// -----------------------------------------
// 1フレームごとの更新処理
// -----------------------------------------
void Player::Update()
{
    animator->Update(); // アニメーション進行

    // ▼ 現在の状態に応じて更新処理を分岐
    switch (state)
    {
    case ST_NORMAL:
        UpdateNormal();
        break;

    case ST_ATTACK1:
    case ST_ATTACK2:
    case ST_ATTACK3:
        UpdateAttack();   // 攻撃関連はまとめて UpdateAttack が分岐
        break;

    case ST_JUMP:
        UpdateJump();     // ジャンプ中
        break;

    case  ST_JUMPATTACK1:
        UpdateJumpAttack();
        break;
    }

    // ▼ 地面との衝突処理
    Stage* stage = FindGameObject<Stage>();
    VECTOR hit;
    VECTOR pos1;
    VECTOR pos2;

    bool jumpState = (state == ST_JUMP) || (state == ST_JUMPATTACK1);

    // --- ジャンプ中は上昇中だけ判定しない ---
    if (jumpState)
    {
        if (velocityY > 0.0f)
        {
            // 上昇中 → 地面判定なし
            isGrounded = false;
        }
        else
        {
            // ▼ 下降中 → 地面チェックあり
            pos1 = transform.position + VGet(0, 100, 0);
            pos2 = transform.position + VGet(0, -100, 0);

            if (stage->CollideLine(pos1, pos2, &hit))
            {
                float distanceToGround = transform.position.y - hit.y;

                if (distanceToGround <= 1.0f)
                {
                    transform.position.y = hit.y; // 地面に吸着
                    velocityY = 0.0f;
                    isGrounded = true;

                    if (state == ST_JUMPATTACK1)
                    {
                        state = ST_NORMAL;
                        animator->Play(A_NEUTRAL);
                    }
                    else if (state == ST_JUMP)
                    {
                        state = ST_NORMAL;
                        animator->Play(A_NEUTRAL);
                    }
                }
            }
        }
    }
    else
    {
        // ▼ 通常時の地面判定
        pos1 = transform.position + VGet(0, 100, 0);
        pos2 = transform.position + VGet(0, -100, 0);

        if (stage->CollideLine(pos1, pos2, &hit))
        {
            transform.position.y = hit.y;
            velocityY = 0.0f;
            isGrounded = true;
        }
    }

    // ▼ カメラにプレイヤーの位置を通知
    camera->SetPlayerPosition(transform.position);
}

// -----------------------------------------
// 描画処理
// -----------------------------------------
void Player::Draw()
{
    Object3D::Draw(); // モデル描画

    // 移動方向のデバッグ線（赤）
    DrawLine3D(transform.position + moveVec * 100, transform.position, GetColor(255, 0, 0));
}

// -----------------------------------------
// 通常状態の更新（移動・回転・攻撃開始など）
// -----------------------------------------
void Player::UpdateNormal()
{
    UpdateRotationFromMouse(); // マウス回転処理

    VECTOR3 inputVec(0, 0, 0); // 入力ベクトル初期化

    // ▼ キーボード入力を入力ベクトルへ反映
    if (CheckHitKey(KEY_INPUT_W)) inputVec.z += 1.0f;
    if (CheckHitKey(KEY_INPUT_S)) inputVec.z -= 1.0f;
    if (CheckHitKey(KEY_INPUT_A)) inputVec.x -= 1.0f;
    if (CheckHitKey(KEY_INPUT_D)) inputVec.x += 1.0f;
    if (CheckHitKey(KEY_INPUT_E)) inputVec.z -= 1.0f;  // E で後退

    // 入力ベクトルを正規化（長さ1に）
    if (inputVec.Size() > 0)
    {
        inputVec = VNorm(inputVec);
    }

    // マウスの X 移動を取得して回転に反映
    int mouseX, mouseY;
    GetMousePoint(&mouseX, &mouseY);

    static int oldMouseX = mouseX;
    float deltaX = (float)(mouseX - oldMouseX);
    oldMouseX = mouseX;

    float rotSpeed = 0.01f;
    transform.rotation.y -= deltaX * rotSpeed; // 向き更新

    // カメラ方向に合わせて入力方向を回転
    VECTOR3 rotatedMove = VECTOR3(
        inputVec.x * cosf(camera->GetTransform().rotation.y) + inputVec.z * sinf(camera->GetTransform().rotation.y),
        0,
        -inputVec.x * sinf(camera->GetTransform().rotation.y) + inputVec.z * cosf(camera->GetTransform().rotation.y)
    );

    moveVec = rotatedMove;

    // 移動しているか判定
    bool isMoving = CheckHitKey(KEY_INPUT_W) || CheckHitKey(KEY_INPUT_A) ||
        CheckHitKey(KEY_INPUT_S) || CheckHitKey(KEY_INPUT_D) || CheckHitKey(KEY_INPUT_E);

    if (CheckHitKey(KEY_INPUT_E))
    {
        // ▼ 後退
        animator->Play(A_BACKRUN);
        transform.rotation.y = atan2f(moveVec.x, moveVec.z);
        transform.position += moveVec * 15.0f;  // 後退速度速め
    }
    else if (isMoving)
    {
        // ▼ 前進・横移動
        animator->Play(A_RUN);
        transform.rotation.y = atan2f(-moveVec.x, -moveVec.z);
        transform.position += moveVec * 5.0f;   // 通常移動
    }
    else
    {
        // ▼ 停止時は待機アニメ
        animator->Play(A_NEUTRAL);
    }

    // -------------------------
    // ▼ ジャンプ
    // -------------------------
    if (isGrounded && IsKeyPressed(KEY_INPUT_SPACE))
    {
        velocityY = 10.0f;  // 上方向へ飛ぶ
        isGrounded = false;
        state = ST_JUMP;
        animator->Play(A_JUMP);
    }

    // -------------------------
    // ▼ 攻撃開始（左クリック）
    // -------------------------
    if (IsMouseClicked(0))
    {
        animator->Play(A_ATTACK1); // 1段目再生
        attackNext = false;        // 次攻撃の予約リセット
        state = ST_ATTACK1;        // 状態変更
    }
    // ▼ Rキーでジャンプ攻撃
    if (IsKeyPressed(KEY_INPUT_R))
    {
        animator->Play(A_JUMPATTACK1); // ジャンプ攻撃アニメ再生
        state = ST_JUMPATTACK1;        // 状態変更
        attackNext = false;           // コンボリセット
        velocityY = 10.0f;             // 上方向へ少し飛ばす（ジャンプ感を出す）
        isGrounded = false;
    }
}

// -----------------------------------------
// ジャンプ中の処理
// -----------------------------------------
void Player::UpdateJump()
{
    UpdateRotationFromMouse();

    velocityY += gravity;              // ▼ 重力で落下
    transform.position.y += velocityY; // Y方向の移動反映

    // ▼ 空中移動（WASD）
    VECTOR3 inputVec(0, 0, 0);
    if (CheckHitKey(KEY_INPUT_W)) inputVec.z += 1.0f;
    if (CheckHitKey(KEY_INPUT_S)) inputVec.z -= 1.0f;
    if (CheckHitKey(KEY_INPUT_A)) inputVec.x -= 1.0f;
    if (CheckHitKey(KEY_INPUT_D)) inputVec.x += 1.0f;

    if (inputVec.Size() > 0)
    {
        inputVec = VNorm(inputVec);

        VECTOR3 rotatedMove(
            inputVec.x * cosf(camera->GetTransform().rotation.y) + inputVec.z * sinf(camera->GetTransform().rotation.y),
            0,
            -inputVec.x * sinf(camera->GetTransform().rotation.y) + inputVec.z * cosf(camera->GetTransform().rotation.y)
        );

        moveVec = rotatedMove;
        transform.position += moveVec * 3.0f;
    }
}

// -----------------------------------------
// マウスでプレイヤーの向きを更新する
// -----------------------------------------
void Player::UpdateRotationFromMouse()
{
    int mouseX, mouseY;
    GetMousePoint(&mouseX, &mouseY);

    static int oldMouseX = mouseX;
    float deltaX = (float)(mouseX - oldMouseX);
    oldMouseX = mouseX;

    float rotSpeed = 0.01f;          // 回転感度
    transform.rotation.y += deltaX * rotSpeed;
}

// -----------------------------------------
// 攻撃全体の分岐処理（実際は各攻撃関数へ転送）
// -----------------------------------------
void Player::UpdateAttack()
{
    switch (state)
    {
    case ST_ATTACK1:
        UpdateAttack1();
        break;

    case ST_ATTACK2:
        UpdateAttack2();
        break;

    case ST_ATTACK3:
        UpdateAttack3();
        break;
    }
}

// -----------------------------------------
// ▼ コンボ1段目
// -----------------------------------------
void Player::UpdateAttack1()
{
    float currentFrame = animator->GetCurrentFrame(); // 今の再生フレーム
    float totalFrame = animator->GetTotalFrame();      // アニメ全体フレーム

    float comboStart = totalFrame * 0.3f; // 30% からコンボ受付
    float comboEnd = totalFrame * 0.7f; // 70% まで

    // ▼ コンボ受付（クリック）
    if (currentFrame >= comboStart && currentFrame <= comboEnd)
    {
        if (IsMouseClicked(0))
            attackNext = true; // 次の攻撃を予約
    }

    // ▼ 攻撃中の前進
    transform.position += GetForward() * -1.5f;

    // ▼ 次の攻撃へ遷移
    if (currentFrame >= comboEnd && attackNext)
    {
        animator->Play(A_ATTACK2);
        attackNext = false;
        state = ST_ATTACK2;
    }

    // ▼ アニメ終了 → 通常へ
    if (animator->IsFinish())
    {
        state = ST_NORMAL;
        attackNext = false;
    }
}

// -----------------------------------------
// ▼ コンボ2段目
// -----------------------------------------
void Player::UpdateAttack2()
{
    float currentFrame = animator->GetCurrentFrame();
    float totalFrame = animator->GetTotalFrame();

    float comboStart = totalFrame * 0.25f; // 25%
    float comboEnd = totalFrame * 0.75f; // 75%

    if (currentFrame >= comboStart && currentFrame <= comboEnd)
    {
        if (IsMouseClicked(0))
            attackNext = true; // 次へつなぐ
    }

    // ▼ 2段目の前進は少し強め
    transform.position += GetForward() * -1.8f;

    // ▼ 3段目へ遷移
    if (currentFrame >= comboEnd && attackNext)
    {
        animator->Play(A_ATTACK3);
        attackNext = false;
        state = ST_ATTACK3;
    }

    if (animator->IsFinish())
    {
        state = ST_NORMAL;
        attackNext = false;
    }
}

// -----------------------------------------
// ▼ コンボ3段目（フィニッシュ）
// -----------------------------------------
void Player::UpdateAttack3()
{
    float currentFrame = animator->GetCurrentFrame();
    float totalFrame = animator->GetTotalFrame();

    // 3段目は入力受付なし

    // ▼ フィニッシュ攻撃は前進を強く
    transform.position += GetForward() * -2.0f;

    // ▼ 終わったら通常へ
    if (animator->IsFinish())
    {
        state = ST_NORMAL;
        attackNext = false;
    }
}

void Player::UpdateJumpAttack()
{
    UpdateRotationFromMouse();

    velocityY += gravity;              // ▼ 重力で落下
    transform.position.y += velocityY; // Y方向の移動反映

    // ▼ 空中移動（WASD）
    VECTOR3 inputVec(0, 0, 0);
    if (CheckHitKey(KEY_INPUT_W)) inputVec.z += 1.0f;
    if (CheckHitKey(KEY_INPUT_S)) inputVec.z -= 1.0f;
    if (CheckHitKey(KEY_INPUT_A)) inputVec.x -= 1.0f;
    if (CheckHitKey(KEY_INPUT_D)) inputVec.x += 1.0f;

    if (inputVec.Size() > 0)
    {
        inputVec = VNorm(inputVec);

        // カメラ方向に合わせる
        VECTOR3 rotatedMove = VECTOR3(
            inputVec.x * cosf(camera->GetTransform().rotation.y) + inputVec.z * sinf(camera->GetTransform().rotation.y),
            0,
            -inputVec.x * sinf(camera->GetTransform().rotation.y) + inputVec.z * cosf(camera->GetTransform().rotation.y)
        );

        moveVec = rotatedMove;

        transform.position += moveVec * 3.0f; // 空中の移動

    }


    if (animator->IsFinish())
    {
        state = ST_NORMAL;
        animator->Play(A_NEUTRAL);
    }
}



// プレイヤーの
// "前方向ベクトw
VECTOR3 Player::GetForward() const
{
    float ry = transform.rotation.y;
    return VGet(sinf(ry), 0, cosf(ry));
}

