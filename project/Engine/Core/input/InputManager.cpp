#include "PCH.h"
#include "InputManager.h"
#include "RawInput.h"
#include "GamePad.h"

InputManager* InputManager::GetInstance() {
	static InputManager instance;
	return &instance;
}

void InputManager::Initialize (HWND hwnd) {
	rawInput_ = std::make_unique<RawInput> ();
	rawInput_->Initialize (hwnd);

	gamePad_ = std::make_unique<GamePad> ();
	gamePad_->Initialize ();
}

void InputManager::Update () {
	gamePad_->Update ();
}

void InputManager::EndFrame () {
	rawInput_->EndFrame ();
}