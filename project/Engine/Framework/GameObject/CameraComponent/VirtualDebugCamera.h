#pragma once
#include "VirtualCameraComponent.h"

// 前方宣言
class InputManager;

class VirtualDebugCamera : public VirtualCameraComponent {
public:
	VirtualDebugCamera() = default;
	~VirtualDebugCamera() override = default;

	void Initialize() override;
	void Update() override;
	void ImGui() override;

	void Serialize(json& j) const override;
	void Deserialize(const json& j) override;

	// アクセッサ
	const char* GetName() const override { return "Virtual Debug  Camera"; }

private:
	float speed_ = 0.3f;
	float sensitivity_ = 0.001f;
	float pitchOver_ = 1.5708f;
	InputManager* input_ = nullptr;
};