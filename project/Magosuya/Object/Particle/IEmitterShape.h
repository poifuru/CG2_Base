#pragma once
#include <string>
#include <random>
#include <json.hpp>
#include "struct.h"
#include "ParticleData.h"

// 前方宣言
struct ParticleBehavior;

class IEmitterShape {
public:
	virtual ~IEmitterShape() = default;

	// パーティクルの初期位置を計算する
	virtual Vector3 GeneratePosition(const EulerTransform& emitterTransform, const ParticleBehavior& behavior, std::mt19937& randomEngine) = 0;

	// 形状ごとのImGui表示
	virtual void ImGui() = 0;

	// シリアライズ（保存・ロード）
	virtual void SaveConfig(nlohmann::json& jsonOut) const = 0;
	virtual void LoadConfig(const nlohmann::json& jsonIn) = 0;

	// 形状の名前（UI等で識別用）
	virtual std::string GetName() const = 0;
};

// 共通ヘルパー関数（ラムダ式は使わない）
inline float ApplyEmitterRandomRange(std::mt19937& randomEngine, bool isRandom, float minVal, float maxVal) {
	if (!isRandom) {
		return minVal; // 固定値ならminの値をそのまま返す
	}
	if (minVal >= maxVal) return minVal;
	std::uniform_real_distribution<float> dist(minVal, maxVal);
	return dist(randomEngine);
}

// ----------------------------------------------------
// Box 形状
// ----------------------------------------------------
class BoxEmitterShape : public IEmitterShape {
public:
	Vector3 GeneratePosition(const EulerTransform& emitterTransform, const ParticleBehavior& behavior, std::mt19937& randomEngine) override;
	void ImGui() override;
	void SaveConfig(nlohmann::json& jsonOut) const override;
	void LoadConfig(const nlohmann::json& jsonIn) override;
	std::string GetName() const override { return "Box"; }
};

// ----------------------------------------------------
// Cylinder 形状
// ----------------------------------------------------
class CylinderEmitterShape : public IEmitterShape {
public:
	Vector3 GeneratePosition(const EulerTransform& emitterTransform, const ParticleBehavior& behavior, std::mt19937& randomEngine) override;
	void ImGui() override;
	void SaveConfig(nlohmann::json& jsonOut) const override;
	void LoadConfig(const nlohmann::json& jsonIn) override;
	std::string GetName() const override { return "Cylinder"; }
};
