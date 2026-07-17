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
		ID3D12Device* device,
		MyEngine::LowLevel::DescriptorHeapManager* heapManager,
		MyEngine::Rendering::RenderTexture* renderTexture
	);

	// 外部がゲーム画面の状態を知るためのゲッター
	bool IsGameWindowHovered() const { return isGameWindowHovered_; }
	bool IsGameWindowFocused() const { return isGameWindowFocused_; }
	ImVec2 GetGameScreenPos() const { return gameScreenPos_; }
	ImVec2 GetGameScreenSize() const { return gameScreenSize_; }

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

	// デバッグ情報を出力する
	void DrawPerformanceWidget(ID3D12Device* device);

private:
	bool isGameWindowHovered_ = false;
	bool isGameWindowFocused_ = false;
	bool isGameWindowDragging_ = false;
	bool isGizmoActive_ = false;

	// アスペクト比変更用
	int selectedAspectIndex_ = 0; // 0: 16:9, 1: 4:3, 2: 自由 (Free)

	ImVec2 gameScreenPos_ = { 0.0f, 0.0f };
	ImVec2 gameScreenSize_ = { 0.0f, 0.0f };
};
