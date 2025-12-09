#include "PlayScene.h"
#include "Player.h"
#include "Stage.h"
#include "StageObject.h"
#include "Camera.h"

PlayScene::PlayScene()
{
	new Camera();
	new Stage(0);
	new Player(VECTOR3(0, 50, 0), 0);
}

PlayScene::~PlayScene()
{
}

void PlayScene::Update()
{
	if (CheckHitKey(KEY_INPUT_T)) {
		SceneManager::ChangeScene("TITLE");
	}
}

void PlayScene::Draw()
{
	auto objects = FindGameObjects<StageObject>();
	for (auto obj : objects) {
		obj->PostDraw();
	}
	DrawString(0, 0, "PLAY SCENE", GetColor(255, 255, 255));
	DrawString(100, 400, "Push [T]Key To Title", GetColor(255, 255, 255));
}
