#pragma once
#include "IEngine.h"

class ImGuiManager {
public:
	static ImGuiManager* GetInstance() {
		static ImGuiManager instance;
		return &instance;
	}
	~ImGuiManager();

	void Initialize(IEngine* engine);
	void Finalize();
	void Draw();
	void BeginFrame();

	IEngine* GetEngine() const { return engine_; }

private:
	void RenderDockingSpace();

private:
	ImGuiManager() = default;
	ImGuiManager(const ImGuiManager&) = delete;
	ImGuiManager& operator=(const ImGuiManager&) = delete;
	ImGuiManager(ImGuiManager&&) = delete;
	ImGuiManager& operator=(ImGuiManager&&) = delete;

	IEngine* engine_ = nullptr;
};

