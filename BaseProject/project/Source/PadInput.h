#pragma once
#include "../Library/GameObject.h"

class PadInput : public GameObject
{
public:
	PadInput(int id);
	~PadInput();
	void Update() override;
	float LStickX();
	float LStickY();
	float RStickX();
	float RStickY();
	VECTOR2 LStickVec();
	VECTOR2 RStickVec();
	bool Press(int id);
	bool OnPush(int id);
	void Reset();

	enum Button {
		ATTACK = XINPUT_BUTTON_X,
		JUMP = XINPUT_BUTTON_A,
	};
private:
	int pad_id;
	XINPUT_STATE input;
	unsigned char prevButtons[16]; // ‘O‚Ìƒ{ƒ^ƒ“
	int prevMouseX, prevMouseY;
};
