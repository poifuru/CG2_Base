#pragma once

class InputManager; // 前方宣言

class WindowsAPI {
public:
	// シングルトンインスタンスの取得
	static WindowsAPI* GetInstance();

	void Initialize(int32_t width, int32_t height);

	bool ProcessMessage();

	void Finalize();

	HWND GetHwnd() { return hwnd_; }
	int32_t GetWindowWidth() { return windowWidth_; }
	int32_t GetWindowHeight() { return windowHeight_; }
	float GetAspectRatio() const { return static_cast<float>(windowWidth_) / static_cast<float>(windowHeight_); }

private:
	WindowsAPI() = default;
	~WindowsAPI() = default;

	// コピー・移動禁止
	WindowsAPI(const WindowsAPI&) = delete;
	WindowsAPI& operator=(const WindowsAPI&) = delete;
	WindowsAPI(WindowsAPI&&) = delete;
	WindowsAPI& operator=(WindowsAPI&&) = delete;

private:	//staticメンバ関数
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:	//メンバ変数
	WNDCLASS windowClass_{};
	HWND hwnd_ = nullptr;
	int32_t windowWidth_ = 1280;
	int32_t windowHeight_ = 720;
};