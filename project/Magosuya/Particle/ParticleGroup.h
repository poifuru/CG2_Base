#pragma once
#include <vector>
#include <list>
#include "DxCommon.h"
#include "Buffer.h"
#include "ParticleData.h"
#include "MaterialData.h"
#include "RenderSystem.h"
#include "CameraComponent.h"

class IParticleField;

class ParticleGroup {
public:
	ParticleGroup(DxCommon* dxCommon);
	~ParticleGroup();

	void Initialize(const std::string& name);
	void Update(const CameraData& cameraData);
	void Draw();
	void ImGui();

	// パーティクルを追加する（エミッターから呼ばれる）
	void AddParticle(const ParticleData& particle);

	// このグループに影響を与えるフィールドを登録する
	void AddField(IParticleField* field);

	// テクスチャをセット
	void SetTexture(D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) { textureHandle_ = textureHandle; }

	// 名前を取得
	std::string GetName() const { return name_; }

private:
	DxCommon* dxCommon_ = nullptr;

	// パイプライン設定（子クラスの Initialize で具体的なIDを詰めさせる）
	PSODescriptor psoDesc_{};
	uint8_t layer_ = 1;
	RenderType renderType_ = RenderType::Particle;

	// バッファ
	std::unique_ptr<VertexBuffer<ParticleVertex>> vertexBuffer_;
	std::unique_ptr<IndexBuffer<uint32_t>> indexBuffer_;
	std::unique_ptr<StructuredBuffer<ParticleForGPU>> instancingBuffer_;
	std::unique_ptr<MaterialResource> materialBuffer_;

	// CPUデータ
	std::string name_{};
	MaterialData material_{};
	std::list<ParticleData> particles_;
	std::vector<IParticleField*> fields_; // 適用するフィールドのポインタ配列
	// テクスチャハンドル
	D3D12_GPU_DESCRIPTOR_HANDLE textureHandle_{};

	// ビルボード切り替え用のフラグ
	bool useBillboard_ = true;

	// 出せるパーティクルの最大数
	const uint32_t kMaxParticleNum_ = 10000;
};