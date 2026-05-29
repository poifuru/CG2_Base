#pragma once
#ifndef __HLSL_VERSION
#include <Windows.h>  // HLSL側では __HLSL_VERSION が定義されてないのでスキップできる
#endif
#include <string>
#include <memory>
#include "InputManager.h"

namespace FileUtils {
	// ファイル選択ダイアログを開いて、選択された絶対パスを返す関数
	std::string OpenFileDialog();

	// 絶対パスを「Resources/」からの相対パスに変換するヘルパー関数
	std::string GetRelativePath(const std::string& absolutePath);
}

class InputManager; // 前方宣言

class WindowsAPI {
public:		//公開メソッド
	static WindowsAPI* GetInstance() {
		//初めて呼び出されたときに一回だけ初期化
		static WindowsAPI instance;
		return &instance;
	}

	void Initialize(InputManager* inputManager);
	bool ProcessMessage();
	void Finalize();
	HWND GetHwnd() { return hwnd_; }

	void SetFullscreen(bool fullscreen);
	bool IsFullscreen() const { return isFullscreen_; }

private:
	//コンストラクタを禁止
	WindowsAPI() = default;
	// コピーコンストラクタと代入演算子を禁止
	WindowsAPI(const WindowsAPI&) = delete;
	WindowsAPI& operator=(const WindowsAPI&) = delete;
	WindowsAPI(WindowsAPI&&) = delete;
	WindowsAPI& operator=(WindowsAPI&&) = delete;

private:	//staticメンバ関数
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:		//定数
	//クライアント領域のサイズ
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

private:	//メンバ変数
	//ウィンドウ
	WNDCLASS windowClass_{};
	HWND hwnd_{};

	//ポインタを借りる
	InputManager* inputManager_ = nullptr;

	// フルスクリーン状態
	bool isFullscreen_ = false;
	WINDOWPLACEMENT windowPlacement_ = { sizeof(WINDOWPLACEMENT) };
};