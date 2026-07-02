#include "IEmitterShape.h"
#include "ParticleGroup.h"
#include "imgui.h"
#include <cmath>

// ----------------------------------------------------
// Box 形状
// ----------------------------------------------------
Vector3 BoxEmitterShape::GeneratePosition(const EulerTransform& emitterTransform, const ParticleBehavior& behavior, std::mt19937& randomEngine) {
	float minX = emitterTransform.translate.x - (emitterTransform.scale.x * 0.5f);
	float maxX = emitterTransform.translate.x + (emitterTransform.scale.x * 0.5f);

	float minY = emitterTransform.translate.y - (emitterTransform.scale.y * 0.5f);
	float maxY = emitterTransform.translate.y + (emitterTransform.scale.y * 0.5f);

	float minZ = emitterTransform.translate.z - (emitterTransform.scale.z * 0.5f);
	float maxZ = emitterTransform.translate.z + (emitterTransform.scale.z * 0.5f);

	Vector3 pos;
	pos.x = ApplyEmitterRandomRange(randomEngine, behavior.isRandomTranslate, minX, maxX);
	pos.y = ApplyEmitterRandomRange(randomEngine, behavior.isRandomTranslate, minY, maxY);
	pos.z = ApplyEmitterRandomRange(randomEngine, behavior.isRandomTranslate, minZ, maxZ);
	return pos;
}

void BoxEmitterShape::ImGui() {
	// Box形状個別のデバッグ設定が必要ならここに記述
}

void BoxEmitterShape::SaveConfig(nlohmann::json& jsonOut) const {
	// Box形状自体には個別のパラメータ保存は不要
}

void BoxEmitterShape::LoadConfig(const nlohmann::json& jsonIn) {
	// Box形状自体には個別のパラメータロードは不要
}

// ----------------------------------------------------
// Cylinder 形状
// ----------------------------------------------------
Vector3 CylinderEmitterShape::GeneratePosition(const EulerTransform& emitterTransform, const ParticleBehavior& behavior, std::mt19937& randomEngine) {
	Vector3 pos;

	if (behavior.isRandomTranslate) {
		// 円柱内の一様ランダムサンプリング（ラムダ式は使わない）
		float theta = ApplyEmitterRandomRange(randomEngine, true, 0.0f, 2.0f * 3.14159265f);
		float r = ApplyEmitterRandomRange(randomEngine, true, 0.0f, 1.0f);
		float t = std::sqrt(r);

		float rx = emitterTransform.scale.x * 0.5f;
		float rz = emitterTransform.scale.z * 0.5f;

		pos.x = emitterTransform.translate.x + std::cos(theta) * t * rx;
		pos.z = emitterTransform.translate.z + std::sin(theta) * t * rz;
	} else {
		// ランダムでない場合は左端に縮退
		pos.x = emitterTransform.translate.x - (emitterTransform.scale.x * 0.5f);
		pos.z = emitterTransform.translate.z - (emitterTransform.scale.z * 0.5f);
	}

	float minY = emitterTransform.translate.y - (emitterTransform.scale.y * 0.5f);
	float maxY = emitterTransform.translate.y + (emitterTransform.scale.y * 0.5f);
	pos.y = ApplyEmitterRandomRange(randomEngine, behavior.isRandomTranslate, minY, maxY);

	return pos;
}

void CylinderEmitterShape::ImGui() {
	// Cylinder形状個別のデバッグ設定が必要ならここに記述
}

void CylinderEmitterShape::SaveConfig(nlohmann::json& jsonOut) const {
	// Cylinder形状自体には個別のパラメータ保存は不要
}

void CylinderEmitterShape::LoadConfig(const nlohmann::json& jsonIn) {
	// Cylinder形状自体には個別のパラメータロードは不要
}
