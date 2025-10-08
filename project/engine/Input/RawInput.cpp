#include "RawInput.h"
#include "../utility/function.h"
#include <hidusage.h>

void RawInput::Initialize (HWND hwnd) {
    RAWINPUTDEVICE rid[2];

    // キーボード
    rid[0].usUsagePage = 0x01; // 汎用デスクトップコントロール
    rid[0].usUsage = 0x06;     // キーボード
    rid[0].dwFlags = RIDEV_INPUTSINK; // フォーカス外でも入力取得
    rid[0].hwndTarget = hwnd;

    // マウス
    rid[1].usUsagePage = 0x01;
    rid[1].usUsage = 0x02;     // マウス
    rid[1].dwFlags = RIDEV_INPUTSINK;
    rid[1].hwndTarget = hwnd;

    if (!RegisterRawInputDevices (rid, 2, sizeof (rid[0]))) {
        MessageBoxA (hwnd, "RawInput 登録失敗", "Error", MB_OK);
    }
    else {
        OutputDebugStringA ("RawInput 登録成功\n");
    }
}

void RawInput::Update (LPARAM lParam) {
    UINT size = 0;
    GetRawInputData ((HRAWINPUT)lParam, RID_INPUT, nullptr, &size, sizeof (RAWINPUTHEADER));

    std::vector<BYTE> buffer (size);
    if (GetRawInputData ((HRAWINPUT)lParam, RID_INPUT, buffer.data (), &size, sizeof (RAWINPUTHEADER)) != size)
        return;

    RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(buffer.data ());

    // キーボード入力
    if (raw->header.dwType == RIM_TYPEKEYBOARD) {
        const RAWKEYBOARD& kb = raw->data.keyboard;
        USHORT key = kb.VKey;
        bool down = !(kb.Flags & RI_KEY_BREAK);
        if (key < 256) keys_[key] = down;
    }

    // マウス入力
    if (raw->header.dwType == RIM_TYPEMOUSE) {
        const RAWMOUSE& ms = raw->data.mouse;
        mouseDeltaX_ = ms.lLastX;
        mouseDeltaY_ = ms.lLastY;
    }
}

bool RawInput::Push (unsigned short key) const {
    return keys_[key];
}

bool RawInput::Trigger (unsigned short key) const {
    return (keys_[key] && !preKeys_[key]);
}

bool RawInput::Release (unsigned short key) const {
    return (!keys_[key] && preKeys_[key]);
}

void RawInput::EndFrame () {
    //preKeysの状態を更新
    preKeys_ = keys_;

    mouseDeltaX_ = 0;
    mouseDeltaY_ = 0;
}