#pragma once
#include <memory>
#include <vector>
#include "ParticleGroup.h"
#include "ParticleEmitter.h"
#include "IParticleField.h"

class ParticleSystem {
public:
	ParticleSystem(DxCommon* dxCommon);
	~ParticleSystem() = default;

	void Initialize(D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);
	void Update(const CameraData& cameraData);
	void Draw();

	// 外部のグローバルなフィールド（ステージの風など）を適用したい場合用
	void AddExternalField(IParticleField* field);

	// エミッターやグループのパラメータ調整用ゲッター（ImGui用など）
	ParticleGroup* GetGroup() { return group_.get(); }

private:
	DxCommon* dxCommon_ = nullptr;

	std::unique_ptr<ParticleGroup> group_;
	std::unique_ptr<ParticleEmitter> emitter_;

	// このシステム固有のフィールド（例：重力など）をまとめて管理
	std::vector<std::unique_ptr<IParticleField>> localFields_;
};