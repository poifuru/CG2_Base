#pragma once

namespace MyEngine::LowLevel {
	class Engine;
}

class EditorManager {
public:
	static EditorManager* GetInstance() {
		static EditorManager instance;
		return &instance;
	}
	~EditorManager() = default;

	// 毎フレームImGuiManagerで呼び出す
	void UpdateAndDraw(MyEngine::LowLevel::Engine* engine);

	// 外部がゲーム画面の状態を知るためのゲッター
	bool IsGameWindowHovered() const { return isGameWindowHovered_; }
	bool IsGameWindowFocused() const { return isGameWindowFocused_; }

	// ギズモがアクティブかどうかを設定・取得する
	void SetGizmoActive(bool active) { isGizmoActive_ = active; }
	bool IsGizmoActive() const { return isGizmoActive_; }

private:
	EditorManager() = default;
	EditorManager(const EditorManager&) = delete;
	EditorManager& operator=(const EditorManager&) = delete;

	// 各ウィンドウの描画関数を小分けにする
	void DrawGameWindow(MyEngine::LowLevel::Engine* engine);

private:
	bool isGameWindowHovered_ = false;
	bool isGameWindowFocused_ = false;
	bool isGameWindowDragging_ = false;
	bool isGizmoActive_ = false;
};
