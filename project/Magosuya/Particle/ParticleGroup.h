#pragma once
#include <vector>
#include <list>
#include "DxCommon.h"
#include "Buffer.h"
#include "ParticleData.h"
#include "MaterialData.h"
#include "RenderSystem.h"
#include "CameraComponent.h"

// パーティクルごとに異なる粒の挙動パラメータ
struct ParticleBehavior {
	Vector4 minColor = { 0.0f, 0.0f, 0.0f, 0.0f };
	Vector4 maxColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	Vector3 minVelocity = { -1.0f, -1.0f, -1.0f };
	Vector3 maxVelocity = {  1.0f,  1.0f,  1.0f };
	float minLifeTime = 1.0f;
	float maxLifeTime = 3.0f;
};

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

	// このグループに影響を与えるフィールドを登録・解除
	void AddField(IParticleField* field);
	bool HasField(IParticleField* field);
	void RemoveField(IParticleField* field);

	// テクスチャをセット
	void SetTexture(D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) { textureHandle_ = textureHandle; }

	// 名前を変更・取得
	void SetName(const std::string& name) { name_ = name; }
	std::string GetName() const { return name_; }

	// ビルボードの変更・取得
	void SetBillBoard(bool flag) { useBillboard_ = flag; }
	bool GetBillBoard() { return useBillboard_; }

	// エミッターが粒子を生成するときに、このルールを読み取れるようにゲッターを用意
	const ParticleBehavior& GetBehavior() const { return behavior_; }

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
	ParticleBehavior behavior_;
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