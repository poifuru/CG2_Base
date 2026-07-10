#pragma once

namespace MyEngine::LowLevel {
	class DescriptorHeapManager;
}

namespace MyEngine::Rendering {
	class RenderTexture;
}

class EditorManager {
public:
	static EditorManager* GetInstance() {
		static EditorManager instance;
		return &instance;
	}
	~EditorManager() = default;

	// 毎フレームImGuiManagerで呼び出す
	void UpdateAndDraw(
		MyEngine::LowLevel::DescriptorHeapManager* heapManager,
		MyEngine::Rendering::RenderTexture* renderTexture
	);

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
	void DrawGameWindow(
		MyEngine::LowLevel::DescriptorHeapManager* heapManager,
		MyEngine::Rendering::RenderTexture* renderTexture
	);

private:
	bool isGameWindowHovered_ = false;
	bool isGameWindowFocused_ = false;
	bool isGameWindowDragging_ = false;
	bool isGizmoActive_ = false;
};
