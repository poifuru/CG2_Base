#pragma once
#include "Component.h"

// 全ての仮想カメラの親クラス
class VirtualCameraComponent : public Component {
public:
	VirtualCameraComponent() = default;
	virtual ~VirtualCameraComponent() override;

	void Initialize() override;
	void ImGui() override;
	void Serialize(json& j) const override;
	void Deserialize(const json& j) override;

	// --- アクセッサ --- //
	// 優先度の取得・設定
	int GetPriority() const { return priority_; }
	void SetPriority(int priority) { priority_ = priority; }

	// FOVの取得・設定
	float GetFov() const { return fov_; }
	void SetFov(float fov) { fov_ = fov; }

	// 現在のカメラの位置・回転を取得
	Vector3 GetPosition() const;
	Vector3 GetRotate() const;

private:
	int priority_ = 10;	// 数値が高いほど優先される
	float fov_ = 0.45f;	// 画角
};