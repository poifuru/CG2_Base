#pragma once
#include "Component.h"
#include "LightManager.h"
#include "struct.h"

class LightComponent : public Component {
public:
	LightComponent() = default;
	~LightComponent() override = default;

	void Initialize() override;
	void Update() override;
	void ImGui() override;

	void Serialize(json& j) const override;
	void Deserialize(const json& j) override;

	const char* GetName() const override { return "Light"; }

	// ゲッター・セッター（LightManagerがデータを集約するときに使う）
	LightType GetLightType() const { return type_; }
	Vector4 GetColor() const { return color_; }
	float GetIntensity() const { return intensity_; }

	// Point/Spot用パラメータ
	float GetRadius() const { return radius_; }
	float GetDecay() const { return decay_; }
	float GetDistance() const { return distance_; }
	float GetCosAngle() const { return cosAngle_; }
	Vector2 GetSize() const { return size_; }
	Vector3 GetRight() const { return right_; }
	Vector3 GetUp() const { return up_; }

private:
	LightType type_ = DIRECTIONALLIGHT;
	Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	float intensity_ = 1.0f;
	// 各ライト用のパラメータ
	float radius_ = 10.0f; // PointLight用
	float decay_ = 1.0f;   // Point/Spot用
	float distance_ = 10.0f; // Spot用
	float cosAngle_ = 0.5f; // Spot用
	Vector2 size_ = { 1.0f, 1.0f };	// Rect用
	Vector3 right_{};	// Rect用
	Vector3 up_{};		// Rect用
};