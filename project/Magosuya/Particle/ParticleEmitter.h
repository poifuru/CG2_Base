#pragma once
#include <random>
#include "ParticleData.h"

class ParticleGroup;

class ParticleEmitter {
public:
	ParticleEmitter(ParticleGroup* targetGroup);
	void Update();

private:
	void Emit();

	ParticleGroup* targetGroup_ = nullptr; // 煙用、炎用などのグループへのポインタ
	Emitter emitterData_;

	//生成エンジン
	std::mt19937 randomEngine_;
	//実行ごとに異なる値を取得する
	std::random_device rd_;

	// 乱数分布
	std::uniform_real_distribution<float> rand_;
};