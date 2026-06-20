#pragma once
#include "IEngine.h"

class EditorManager {
public:
	static EditorManager* GetInstance() {
		static EditorManager instance;
		return &instance;
	}
	~EditorManager() = default;

	// 毎フレームImGuiManagerで呼び出す
	void UpdateAndDraw(IEngine* engine);

	// 外部がゲーム画面の状態を知るためのゲッター
	bool IsGameWindowHovered() const { return isGameWindowHovered_; }
	bool IsGameWindowFocused() const { return isGameWindowFocused_; }

private:
	EditorManager() = default;
	EditorManager(const EditorManager&) = delete;
	EditorManager& operator=(const EditorManager&) = delete;

	// 各ウィンドウの描画関数を小分けにする
	void DrawGameWindow(IEngine* engine);
	void DrawViewportWindow();
	void DrawInspectorWindow();

private:
	bool isGameWindowHovered_ = false;
	bool isGameWindowFocused_ = false;
	bool isGameWindowDragging_ = false;
};
