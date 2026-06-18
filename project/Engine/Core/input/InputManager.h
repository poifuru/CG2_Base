#pragma once
#include <Windows.h>
#include <memory>
#include "RawInput.h"
#include "GamePad.h"

class InputManager {
public:
	InputManager() = default;
	~InputManager() = default;

	void Initialize (HWND hwnd);

	void Update ();

	//preKeys更新用
	void EndFrame ();

	RawInput* GetRawInput () { return rawInput_.get (); }
	GamePad* GetGamePad () { return gamePad_.get (); }

private:
	// コピー・移動禁止
	InputManager(const InputManager&) = delete;
	InputManager& operator=(const InputManager&) = delete;
	InputManager(InputManager&&) = delete;
	InputManager& operator=(InputManager&&) = delete;

private://メンバ変数
	std::unique_ptr<RawInput> rawInput_;
	std::unique_ptr<GamePad> gamePad_;
};