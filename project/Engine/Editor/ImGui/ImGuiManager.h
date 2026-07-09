#pragma once

class Engine;

class ImGuiManager {
public:
	static ImGuiManager* GetInstance() {
		static ImGuiManager instance;
		return &instance;
	}
	~ImGuiManager();

	void Initialize(Engine* engine);
	void Finalize();
	void Draw();
	void BeginFrame();

	Engine* GetEngine() const { return engine_; }

private:
	void RenderDockingSpace();

private:
	ImGuiManager() = default;
	ImGuiManager(const ImGuiManager&) = delete;
	ImGuiManager& operator=(const ImGuiManager&) = delete;
	ImGuiManager(ImGuiManager&&) = delete;
	ImGuiManager& operator=(ImGuiManager&&) = delete;

	Engine* engine_ = nullptr;
};

