#pragma once

class ImGuiManager {
public:
	static ImGuiManager* GetInstance () {
		//初めて呼び出されたときに一回だけ初期化
		static ImGuiManager instance;
		return &instance;
	}
	~ImGuiManager ();

	void Initialize ();
	void Draw ();
	void BeginFrame ();

	/*bool IsGameWindowHovered() const { return isGameWindowHovered_; }
	bool IsGameWindowFocused() const { return isGameWindowFocused_; }*/

private:
	void RenderDockingSpace();

private:
	//コンストラクタを禁止
	ImGuiManager () = default;
	// コピーコンストラクタと代入演算子を禁止
	ImGuiManager (const ImGuiManager&) = delete;
	ImGuiManager& operator=(const ImGuiManager&) = delete;
	ImGuiManager (ImGuiManager&&) = delete;
	ImGuiManager& operator=(ImGuiManager&&) = delete;

	/*bool isGameWindowHovered_ = false;
	bool isGameWindowFocused_ = false;
	bool isGameWindowDragging_ = false;*/
};

