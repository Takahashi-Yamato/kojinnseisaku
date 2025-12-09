#pragma once

/// <summary>
/// Windowの定義をします
/// </summary>
#define FIX_FRAME_RATE  // フレームレートを固定にする
namespace Screen
{
	static const int WIDTH = 1280;
	static const int HEIGHT = 720;
	static const BOOL WINDOW_MODE = TRUE;
	static const char* WINDOW_NAME = "project";
	static const float WINDOW_EXTEND = 1.0f;
	static const float FRAME_RATE = 60.0f;
};
