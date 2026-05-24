#include "ParticleGroup.h"
#include "IParticleField.h"

ParticleGroup::ParticleGroup(DxCommon* dxCommon) {
	dxCommon_ = dxCommon;
}

ParticleGroup::~ParticleGroup() {

}

void ParticleGroup::Initialize() {

}

// 更新処理の抜粋
void ParticleGroup::Update(float deltaTime) {
	std::vector<ParticleForGPU> updateData;
	updateData.reserve(particles_.size());

	for (auto it = particles_.begin(); it != particles_.end();) {
		if (it->currentTime >= it->lifeTime) {
			it = particles_.erase(it);
			continue;
		}

		// 1. 各種フィールドの効果を適用
		it->acceleration = { 0.0f, 0.0f, 0.0f }; // 毎フレーム初期化
		for (size_t i = 0; i < fields_.size(); ++i) {
			fields_[i]->Apply(*it);
		}

		// 2. 物理計算（加速度 -> 速度 -> 位置）
		it->velocity += it->acceleration * deltaTime;
		it->transform.translate += it->velocity * deltaTime;
		it->currentTime += deltaTime;

		// 3. インスタンシング用バッファに詰める処理（省略）
		// ...

		++it;
	}

	// まとめてGPUバッファに転送！
	instancingBuffer_.Update(updateData);
}

void ParticleGroup::AddParticle(const ParticleData& particle) {
	if (particles_.size() < kMaxParticleNum_) {
		particles_.push_back(particle);
	}
}