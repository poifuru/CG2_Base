#pragma once
#include "ParticleEmitter.h"
#include "ParticleGroup.h"
#include "Deltatime.h"

ParticleEmitter::ParticleEmitter(ParticleGroup* targetGroup) {
	targetGroup_ = targetGroup;
	// シードの設定はここ（初期化時）で1回だけ！
	randomEngine_.seed(rd_());

	// 自身のデータの初期化
	emitterData_.transform = { {}, {}, {} };
	emitterData_.count = 10;
	emitterData_.frequency = 3.0f;
	emitterData_.frequencyTime = 0.0f;
}

void ParticleEmitter::Update(float deltaTime) {

}

void ParticleEmitter::Emit() {
	ParticleData data;
	// 乱数エンジンをそのまま分布に渡す
	data.transform = { {}, {},
		{emitterData_.transform.translate.x + rand_(randomEngine_),
		emitterData_.transform.translate.y + rand_(randomEngine_),
		emitterData_.transform.translate.z + rand_(randomEngine_)}
	};
	data.velocity = { rand_(randomEngine_), rand_(randomEngine_), rand_(randomEngine_) };
	data.acceleration = {};
	data.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	data.lifeTime = 3.0f;
	data.currentTime = 0.0f;

	// ターゲットのグループに登録する
	targetGroup_->AddParticle(data);
}