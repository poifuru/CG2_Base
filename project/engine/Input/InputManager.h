#pragma once
#include "../../header/WinSupport.h"
#include "RawInput.h"
#include "GamePad.h"

class InputManager {
public:	//メンバ関数
	static InputManager* GetInstance ();

	void Initialize (HWND hwnd);
	void Update (LPARAM lparam);
	//preKeys更新用
	void EndFrame ();

	RawInput* GetRawInput () { return &rawInput_; }

private://メンバ変数
	RawInput rawInput_;
};

