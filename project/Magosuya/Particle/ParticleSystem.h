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

	void Initialize();
	void Update(const CameraData& cameraData);
	void Draw();
	void ImGui();

	// 動的に要素を追加する関数
	void AddEmitter(const std::string& name);
	void AddGroup(const std::string& name, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);
	void AddField(std::unique_ptr<IParticleField> field);

private:
	DxCommon* dxCommon_ = nullptr;

	std::vector<std::unique_ptr<ParticleGroup>> groups_;
	std::vector<std::unique_ptr<ParticleEmitter>> emitters_;
	std::vector<std::unique_ptr<IParticleField>> fields_;

	// ImGuiの選択状態を管理する変数
	int currentSelectedEmitter_ = 0;
	int currentSelectedGroup_ = 0;
	int currentSelectedField_ = 0;
};