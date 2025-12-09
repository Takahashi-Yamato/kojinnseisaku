#include "PadInput.h"
#include <assert.h>
#define USE_MOUSE_CAMERA // マウスで右スティックの変わり
#define USE_KEYBOARD_MOVE // キーボードで移動

static const int MAX = 32767;
static const int LIMIT = MAX / 5;

PadInput::PadInput(int id)
{
    pad_id = id;
    DontDestroyOnSceneChange();
#ifdef USE_MOUSE_CAMERA // マウスで右スティックの変わり
    GetMousePoint(&prevMouseX, &prevMouseY);
#endif
}

PadInput::~PadInput()
{
}

float StickValue(int stick) {
    if (-LIMIT < stick && stick < LIMIT)
    {
        return 0.0f;
    }
    else if (stick > 0)
    {
        float ret = (float)(stick - LIMIT) / (MAX - LIMIT);
        if (ret > 1.0f) {
            ret = 1.0f;
        }
        return ret;
    }
    else
    {
        float ret = (float)(stick + LIMIT) / (MAX - LIMIT);
        if (ret < -1.0f) {
            ret = -1.0f;
        }
        return ret;
    }
}

float PadInput::LStickX()
{
    return StickValue(input.ThumbLX);
}

float PadInput::LStickY()
{
    return StickValue(input.ThumbLY);
}

float PadInput::RStickX()
{
    return StickValue(input.ThumbRX);
}

float PadInput::RStickY()
{
    return StickValue(input.ThumbRY);
}

VECTOR2 StickVec(int x, int y)
{
    float size = sqrtf((float)x * x + (float)y * y);
    if (size < LIMIT)
    {
        return VECTOR2(0, 0);
    }
    else if (size > MAX)
    {
        return VECTOR2(x / size, y / size);
    }
    else
    {
        return VECTOR2(x / (float)MAX, y / (float)MAX);
    }
}

VECTOR2 PadInput::LStickVec()
{
    VECTOR2 ret = VECTOR2(LStickX(), LStickY());
    if (CheckHitKey(KEY_INPUT_W)) {
        ret += VECTOR2(0, 1);
    }
    if (CheckHitKey(KEY_INPUT_S)) {
        ret += VECTOR2(0, -1);
    }
    if (CheckHitKey(KEY_INPUT_D)) {
        ret += VECTOR2(1, 0);
    }
    if (CheckHitKey(KEY_INPUT_A)) {
        ret += VECTOR2(-1, 0);
    }
    if (ret.Size() > 1.0f) {
        ret = ret.Normalize();
    }
    return ret;
}

VECTOR2 PadInput::RStickVec()
{
    return StickVec(input.ThumbRX, input.ThumbRY);
}

bool PadInput::Press(int id)
{
    assert(id < 16);
    return input.Buttons[id] > 0;
}

bool PadInput::OnPush(int id)
{
    assert(id < 16);
    return input.Buttons[id] > 0 && prevButtons[id] == 0;
}

void PadInput::Reset()
{
    GetMousePoint(&prevMouseX, &prevMouseY);
    for (int i = 0; i < 16; i++)
    {
        prevButtons[i] = 1;
    }
}

inline int clamp(int v, int minV, int maxV)
{
    if (v < minV)
        return minV;
    if (v > maxV)
        return maxV;
    return v;
}

void PadInput::Update()
{

    for (int i = 0; i < 16; i++)
    {
        prevButtons[i] = input.Buttons[i];
    }
    GetJoypadXInputState(pad_id, &input);

    if (CheckHitKey(KEY_INPUT_M))
    {
        input.Buttons[XINPUT_BUTTON_A] = 1;
    }

#ifdef USE_KEYBOARD_MOVE
#endif
#ifdef USE_MOUSE_CAMERA
    static const int SENSE = 40;
    int mouseX, mouseY;
    GetMousePoint(&mouseX, &mouseY);
    int dragX = (mouseX - prevMouseX) * MAX / SENSE;
    int dragY = (mouseY - prevMouseY) * MAX / SENSE;
    input.ThumbRX = clamp(dragX + input.ThumbRX, -MAX, MAX);
    input.ThumbRY = clamp(dragY + input.ThumbRY, -MAX, MAX);
    prevMouseX = mouseX;
    prevMouseY = mouseY;
#endif
    if (GetMouseInput() & MOUSE_INPUT_LEFT)
    {
        input.Buttons[XINPUT_BUTTON_X] = 1;
    }
}
