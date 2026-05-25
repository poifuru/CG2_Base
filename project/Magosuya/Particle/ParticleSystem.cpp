#include "ParticleSystem.h"
#include "ParticleSystem.h"
#include "ParticleField.h" // GravityField など
#include "imgui.h"

ParticleSystem::ParticleSystem(DxCommon* dxCommon) : dxCommon_(dxCommon) {}

void ParticleSystem::Initialize(D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) {
	// グループの生成と初期化
	group_ = std::make_unique<ParticleGroup>(dxCommon_);
	group_->Initialize();
	group_->SetTexture(textureHandle);

	// エミッターの生成（自分自身のグループへのポインタを渡す）
	emitter_ = std::make_unique<ParticleEmitter>(group_.get());

	// このエフェクト固有のフィールドがあれば生成してグループに登録
	// 例として、全パーティクルシステムにデフォルトで緩い重力をかけてみる
	auto gravity = std::make_unique<GravityField>(Vector3{0.0f, -0.98f, 0.0f});
	group_->AddField(gravity.get());
	localFields_.push_back(std::move(gravity)); // 寿命管理はここで行う
}

void ParticleSystem::Update(const CameraData& cameraData) {
	// エミッターを更新してパーティクルを発生させる
	if (emitter_) {
		emitter_->Update();
	}

	// グループを更新して物理計算＆バッファ転送
	if (group_) {
		group_->Update(cameraData);
	}
}

void ParticleSystem::Draw() {
	// 描画コマンドをRenderSystemに積む
	if (group_) {
		group_->Draw();
	}
}

void ParticleSystem::AddExternalField(IParticleField* field) {
	// ステージの風など、外部のフィールドをグループに中継する
	if (group_) {
		group_->AddField(field);
	}
}