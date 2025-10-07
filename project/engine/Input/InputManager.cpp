#include "InputManager.h"

void InputManager::Initialize (HWND hwnd) {
	rawInput_.Initialize (hwnd);
}

void InputManager::Update (LPARAM lparam) {
	rawInput_.Update (lparam);
}