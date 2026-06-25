#pragma once
#include "VirtualCameraComponent.h"

class VirtualFollowCamera : public VirtualCameraComponent {
public:
	VirtualFollowCamera() = default;
	~VirtualFollowCamera() override = default;

	void Update() override;
	void ImGui() override;

	void ResolveTarget(const std::vector<std::unique_ptr<GameObject>>& gameObjects);

	void Serialize(json& j) const override;
	void Deserialize(const json& j) override;

	// アクセッサ
	const char* GetName() const override { return "Virtual Follow Camera"; }
private:
	std::string targetName_ = "";
	GameObject* target_ = nullptr;

	Vector3 offset_ = { 0.0f, 5.0f, -10.0f }; // ターゲットからの距離
	float delay_ = 0.1f;                      // 追従の遅延 (0 = 遅延なし, 1 = 動かない)
};