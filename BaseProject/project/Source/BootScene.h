#pragma once

#include "../Library/SceneBase.h"

/// <summary>
/// 起動時のシーン
/// 共通で必要なGameObjectを生成して、
/// シーンを変更する
/// </summary>
class BootScene : public SceneBase
{
public:
	BootScene();
	~BootScene();
	void Update() override;
	void Draw() override;
};