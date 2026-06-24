#pragma once

// 前方宣言
class RawInput;
class GamePad;

class InputManager {
public:
	// シングルトンインスタンスの取得
	static InputManager* GetInstance();

	void Initialize (HWND hwnd);

	void Update ();

	//preKeys更新用
	void EndFrame ();

	RawInput* GetRawInput () { return rawInput_.get (); }
	GamePad* GetGamePad () { return gamePad_.get (); }

private:
	InputManager() = default;
	~InputManager() = default;

	// コピー・移動禁止
	InputManager(const InputManager&) = delete;
	InputManager& operator=(const InputManager&) = delete;
	InputManager(InputManager&&) = delete;
	InputManager& operator=(InputManager&&) = delete;

private://メンバ変数
	std::unique_ptr<RawInput> rawInput_;
	std::unique_ptr<GamePad> gamePad_;
};