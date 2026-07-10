#pragma once
#include "ParticleEmitter.h"
#include "ParticleGroup.h"
#include "Deltatime.h"
#include "imgui.h"
#include "Primitive.h"

ParticleEmitter::ParticleEmitter(const std::string& name) {
	name_ = name;
	// シードの設定はここ（初期化時）で1回だけ！
	randomEngine_.seed(rd_());

	// 自身のデータの初期化
	emitterData_.transform = { {1.0f, 1.0f, 1.0f}, {}, {} };
	emitterData_.count = 10;
	emitterData_.frequency = 1.0f;
	emitterData_.frequencyTime = 0.0f;
	shape_ = std::make_unique<BoxEmitterShape>();
}

void ParticleEmitter::Update() {
	emitterData_.frequencyTime += kDeltaTime;
	if(emitterData_.frequency <= emitterData_.frequencyTime && emitterData_.frequency >= 0.0f) {
		// 紐付いているグループの数だけ、それぞれにパーティクルを発射する！
		for(size_t g = 0; g < targetGroups_.size(); ++g) {
			for(uint32_t i = 0; i < emitterData_.count; ++i) {
				Emit(targetGroups_[g]);
			}
		}
		emitterData_.frequencyTime -= emitterData_.frequency;
	}
}

void ParticleEmitter::ImGui() {
#ifdef USEIMGUI
	// 名前の変更
	char nameBuffer[128];
	snprintf(nameBuffer, sizeof(nameBuffer), "%s", name_.c_str());
	if(ImGui::InputText("Emitter Name", nameBuffer, sizeof(nameBuffer))) {
		name_ = nameBuffer;
	}

	ImGui::DragFloat3("Scale", &emitterData_.transform.scale.x, 0.01f);
	ImGui::DragFloat3("Rotate", &emitterData_.transform.rotate.x, 0.01f);
	ImGui::DragFloat3("Translate", &emitterData_.transform.translate.x, 0.01f);
	int tempCount = static_cast<int>(emitterData_.count);
	if(ImGui::DragInt("Emit Count", &tempCount, 1, 0, 10000)) {
		emitterData_.count = tempCount;
	}
	ImGui::DragFloat("Frequency", &emitterData_.frequency, 0.01f, 0.0f, 180.0f);

	// 形状切り替えUI
	if (shape_) {
		std::string currentShape = shape_->GetName();
		if (ImGui::BeginCombo("Shape Type", currentShape.c_str())) {
			if (ImGui::Selectable("Box", currentShape == "Box")) {
				shape_ = std::make_unique<BoxEmitterShape>();
			}
			if (ImGui::Selectable("Cylinder", currentShape == "Cylinder")) {
				shape_ = std::make_unique<CylinderEmitterShape>();
			}
			ImGui::EndCombo();
		}
		shape_->ImGui();
	}

	ImGui::Separator();
	if(ImGui::Button("Emit TargetParticle") && !targetGroups_.empty()) {
		for(size_t i = 0; i < targetGroups_.size(); ++i) {
			Emit(targetGroups_[i]);
		}
	}
#endif
}

void ParticleEmitter::Emit(ParticleGroup* group) {
	if(!group) return;

	const ParticleBehavior& behavior = group->GetBehavior();
	ParticleData data;

	// 基本的なトランスフォーム（Scale, Rotate）はグループの挙動をベースにする
	data.transform = behavior.transform; 

	// Scale
	data.transform.scale.x = ApplyRandomRange(behavior.isRandomScale, behavior.minScale.x, behavior.maxScale.x);
	data.transform.scale.y = ApplyRandomRange(behavior.isRandomScale, behavior.minScale.y, behavior.maxScale.y);
	data.transform.scale.z = ApplyRandomRange(behavior.isRandomScale, behavior.minScale.z, behavior.maxScale.z);

	// Rotate
	data.transform.rotate.x = ApplyRandomRange(behavior.isRandomRotate, behavior.minRotate.x, behavior.maxRotate.x);
	data.transform.rotate.y = ApplyRandomRange(behavior.isRandomRotate, behavior.minRotate.y, behavior.maxRotate.y);
	data.transform.rotate.z = ApplyRandomRange(behavior.isRandomRotate, behavior.minRotate.z, behavior.maxRotate.z);

	// 出現位置の決定：エミッター形状クラス（shape_）に委譲する
	if (shape_) {
		data.transform.translate = shape_->GeneratePosition(emitterData_.transform, behavior, randomEngine_);
	} else {
		data.transform.translate = emitterData_.transform.translate;
	}

	// 速度の決定（グループ側の設定を適用）
	data.velocity.x = ApplyRandomRange(behavior.isRandomVelocity, behavior.minVelocity.x, behavior.maxVelocity.x);
	data.velocity.y = ApplyRandomRange(behavior.isRandomVelocity, behavior.minVelocity.y, behavior.maxVelocity.y);
	data.velocity.z = ApplyRandomRange(behavior.isRandomVelocity, behavior.minVelocity.z, behavior.maxVelocity.z);

	data.acceleration = {};

	// 色の決定
	data.color.x = ApplyRandomRange(behavior.isRandomColor, behavior.maxColor.x, behavior.minColor.x);
	data.color.y = ApplyRandomRange(behavior.isRandomColor, behavior.maxColor.y, behavior.minColor.y);
	data.color.z = ApplyRandomRange(behavior.isRandomColor, behavior.maxColor.z, behavior.minColor.z);
	data.color.w = ApplyRandomRange(behavior.isRandomColor, behavior.maxColor.w, behavior.minColor.w);

	// 寿命の決定
	data.lifeTime = ApplyRandomRange(behavior.isRandomLifeTime, behavior.minLifeTime, behavior.maxLifeTime);
	data.currentTime = 0.0f;

	// そのグループに直接放り込む
	group->AddParticle(data);
}

void ParticleEmitter::SaveConfig(json& jsonOut) const {
	jsonOut["name"] = name_;

	auto& e = jsonOut["emitterData"];
	e["count"] = emitterData_.count;
	e["frequency"] = emitterData_.frequency;
	e["scale"] = { emitterData_.transform.scale.x, emitterData_.transform.scale.y, emitterData_.transform.scale.z };
	e["rotate"] = { emitterData_.transform.rotate.x, emitterData_.transform.rotate.y, emitterData_.transform.rotate.z };
	e["translate"] = { emitterData_.transform.translate.x, emitterData_.transform.translate.y, emitterData_.transform.translate.z };

	if (shape_) {
		e["shapeType"] = shape_->GetName();
		json shapeData = json::object();
		shape_->SaveConfig(shapeData);
		e["shapeData"] = shapeData;
	}

	// このエミッターが狙っているターゲットグループの名前を保存しておく
	nlohmann::json targets = nlohmann::json::array();
	for (size_t i = 0; i < targetGroups_.size(); ++i) {
		targets.push_back(targetGroups_[i]->GetName());
	}
	jsonOut["targetGroups"] = targets;
}

void ParticleEmitter::LoadConfig(const json& jsonIn) {
	// name
	if(jsonIn.contains("name")) name_ = jsonIn["name"];

	if(jsonIn.contains("emitterData")) {
		auto& e = jsonIn["emitterData"];

		// Scale
		emitterData_.transform.scale.x = e["scale"][0];
		emitterData_.transform.scale.y = e["scale"][1];
		emitterData_.transform.scale.z = e["scale"][2];

		// Rotate
		emitterData_.transform.rotate.x = e["rotate"][0];
		emitterData_.transform.rotate.y = e["rotate"][1];
		emitterData_.transform.rotate.z = e["rotate"][2];

		// Translate
		emitterData_.transform.translate.x = e["translate"][0];
		emitterData_.transform.translate.y = e["translate"][1];
		emitterData_.transform.translate.z = e["translate"][2];

		// Count
		emitterData_.count = e["count"];

		// Frequency
		emitterData_.frequency = e["frequency"];

		// 形状のロード
		if (e.contains("shapeType")) {
			std::string shapeType = e["shapeType"];
			if (shapeType == "Cylinder") {
				shape_ = std::make_unique<CylinderEmitterShape>();
			} else {
				shape_ = std::make_unique<BoxEmitterShape>();
			}
			if (e.contains("shapeData")) {
				shape_->LoadConfig(e["shapeData"]);
			}
		} else {
			shape_ = std::make_unique<BoxEmitterShape>();
		}
	}
}

// 1. 指定されたグループをターゲット（発射先）に追加する
void ParticleEmitter::TargetGroup(ParticleGroup* group) {
	if(group == nullptr) return;

	// すでに登録済みなら二重登録しないようにチェック
	if(IsTargeting(group)) return;

	// 配列に追加
	targetGroups_.push_back(group);
}

// 2. 指定されたグループをターゲットから外す
void ParticleEmitter::UntargetGroup(ParticleGroup* group) {
	if(group == nullptr) return;

	// ループ（イテレータ）を回して、一致するポインタを探して削除する
	for(auto it = targetGroups_.begin(); it != targetGroups_.end(); ) {
		if(*it == group) {
			it = targetGroups_.erase(it); // 見つけたら削除して次のイテレータを受け取る
		}
		else {
			++it; // 見つからなければ次に進む
		}
	}
}

// 3. 指定されたグループがすでにターゲットに含まれているか調べる
bool ParticleEmitter::IsTargeting(ParticleGroup* group) const {
	if(group == nullptr) return false;

	// 配列を愚直にループして、同じアドレスのポインタがあるか探す
	for(size_t i = 0; i < targetGroups_.size(); ++i) {
		if(targetGroups_[i] == group) {
			return true; // 見つかった
		}
	}

	return false; // 見つからなかった
}

float ParticleEmitter::ApplyRandomRange(bool isRandom, float minVal, float maxVal) {
	if (!isRandom) {
		return minVal; // 固定値ならminの値をそのまま返す
	}
	if (minVal >= maxVal) return minVal;
	std::uniform_real_distribution<float>::param_type param(minVal, maxVal);
	return rand_(randomEngine_, param);
}
