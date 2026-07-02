#pragma once
#include <random>
#include <memory>
#include <json.hpp>
using namespace nlohmann;
#include "ParticleData.h"
#include "IEmitterShape.h"

class ParticleGroup;

class ParticleEmitter {
public:
	ParticleEmitter(const std::string& name);
	void Update();
	void ImGui();

	void Emit(ParticleGroup* group);

	// 設定の保存・ロード
	void SaveConfig(json& jsonOut)const;
	void LoadConfig(const json& jsonIn);

	// 名前の設定・取得
	void SetName(const std::string& name) { name_ = name; }
	std::string GetName() const { return name_; }

	// 名前を直接書き換えるためのバッファ取得用
	std::string& GetNameRef() { return name_; }
	// ImGui編集用に参照を返す
	Emitter& GetEmitterData() { return emitterData_; }

	// 4枚目のタブでくっつけたり外したりするための関数
	void TargetGroup(ParticleGroup* group);
	void UntargetGroup(ParticleGroup* group);
	bool IsTargeting(ParticleGroup* group) const;

	// 形状の取得・設定用
	IEmitterShape* GetShape() const { return shape_.get(); }
	void SetShape(std::unique_ptr<IEmitterShape> shape) { shape_ = std::move(shape); }

private:
	// 乱数生成ヘルパー関数
	float ApplyRandomRange(bool isRandom, float minVal, float maxVal);

private:
	std::vector<ParticleGroup*> targetGroups_; // このエミッターが発射する先のグループ達
	Emitter emitterData_;
	std::string name_{};
	std::unique_ptr<IEmitterShape> shape_; // 形状を管理するクラス

	//生成エンジン
	std::mt19937 randomEngine_;
	//実行ごとに異なる値を取得する
	std::random_device rd_;

	// 乱数分布
	std::uniform_real_distribution<float> rand_;
};