#pragma once

namespace MyEngine::LowLevel {
	class Engine;
}

class ImGuiManager {
public:
	static ImGuiManager* GetInstance() {
		static ImGuiManager instance;
		return &instance;
	}
	~ImGuiManager();

	void Initialize(MyEngine::LowLevel::Engine* engine);
	void Finalize();
	void Draw();
	void BeginFrame();

	MyEngine::LowLevel::Engine* GetEngine() const { return engine_; }

private:
	void RenderDockingSpace();

private:
	ImGuiManager() = default;
	ImGuiManager(const ImGuiManager&) = delete;
	ImGuiManager& operator=(const ImGuiManager&) = delete;
	ImGuiManager(ImGuiManager&&) = delete;
	ImGuiManager& operator=(ImGuiManager&&) = delete;

	MyEngine::LowLevel::Engine* engine_ = nullptr;
};

