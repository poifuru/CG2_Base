#pragma once
#include "ParticleEmitter.h"
#include "ParticleGroup.h"
#include "Deltatime.h"
#include "imgui.h"

ParticleEmitter::ParticleEmitter(const std::string& name) {
	name_ = name;
	// シードの設定はここ（初期化時）で1回だけ！
	randomEngine_.seed(rd_());

	// 自身のデータの初期化
	emitterData_.transform = { {}, {}, {} };
	emitterData_.count = 10;
	emitterData_.frequency = 1.0f;
	emitterData_.frequencyTime = 0.0f;
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
	if(ImGui::DragInt("Emit Count", &tempCount, 1, 0, 100)) {
		emitterData_.count = tempCount;
	}
	ImGui::DragFloat("Frequency", &emitterData_.frequency, 0.01f, 0.0f, 180.0f);
	ImGui::Separator();
	if(ImGui::Button("Emit TargetParticle") && !targetGroups_.empty()) {
		for(size_t i = 0; i < targetGroups_.size(); ++i) {
			Emit(targetGroups_[i]);
		}
	}
#endif
}

//void ParticleEmitter::ImGuiBehavior() {
//#ifdef USEIMGUI
//	// 名前の変更（std::stringを直接編集できるようにテキスト入力を工夫）
//	// ImGuiのInputTextはchar配列が必要なので一時バッファを使う
//	char nameBuffer[128];
//	snprintf(nameBuffer, sizeof(nameBuffer), "%s", name_.c_str());
//	if(ImGui::InputText("Group/Emitter Name", nameBuffer, sizeof(nameBuffer))) {
//		name_ = nameBuffer; // 入力されたら名前を更新
//	}
//
//	ImGui::Separator();
//	ImGui::Text("パーティクルの挙動設定");
//
//	// 寿命の範囲
//	ImGui::DragFloat2("LifeTime (Min/Max)", &behavior_.minLifeTime, 0.05f, 0.0f, 10.0f);
//
//	// 速度の範囲
//	ImGui::DragFloat3("Velocity Min", &behavior_.minVelocity.x, 0.05f);
//	ImGui::DragFloat3("Velocity Max", &behavior_.maxVelocity.x, 0.05f);
//
//	// カラーの範囲
//	ImGui::ColorEdit4("Color Min", &behavior_.minColor.x);
//	ImGui::ColorEdit4("Color Max", &behavior_.maxColor.x);
//#endif
//}

void ParticleEmitter::Emit(ParticleGroup* group) {
	if(!group) return;

	// 指定されたグループから生成ルール（Behavior）を借りてくる
	const ParticleBehavior& behavior = group->GetBehavior();

	ParticleData data;

	// 指定された範囲（min〜max）でランダムな値を生成する簡易ヘルパー
	// std::uniform_real_distributionの範囲をその場で書き換えて使用
	auto GetRandFloat = [this](float min, float max) {
		if(min >= max) return min;
		std::uniform_real_distribution<float>::param_type param(min, max);
		return rand_(randomEngine_, param);
		};

	// 1. 位置の設定（エミッターのTranslateを基準に少し散らす）
	data.transform = { {1.0f, 1.0f, 1.0f}, {},
		{
			emitterData_.transform.translate.x + GetRandFloat(-0.5f, 0.5f),
			emitterData_.transform.translate.y + GetRandFloat(-0.5f, 0.5f),
			emitterData_.transform.translate.z + GetRandFloat(-0.5f, 0.5f)
		}
	};

	// 2. 挙動パラメータ（behavior_）を元にランダムに決定
	data.velocity = {
		GetRandFloat(behavior.minVelocity.x, behavior.maxVelocity.x),
		GetRandFloat(behavior.minVelocity.y, behavior.maxVelocity.y),
		GetRandFloat(behavior.minVelocity.z, behavior.maxVelocity.z)
	};

	data.acceleration = {};

	data.color = {
		GetRandFloat(behavior.minColor.x, behavior.maxColor.x),
		GetRandFloat(behavior.minColor.y, behavior.maxColor.y),
		GetRandFloat(behavior.minColor.z, behavior.maxColor.z),
		GetRandFloat(behavior.minColor.w, behavior.maxColor.w)
	};

	data.lifeTime = GetRandFloat(behavior.minLifeTime, behavior.maxLifeTime);
	data.currentTime = 0.0f;

	// そのグループに直接放り込む
	group->AddParticle(data);
}

// 1. 指定されたグループをターゲット（発射先）に追加する
void ParticleEmitter::TargetGroup(ParticleGroup* group) {
	if (group == nullptr) return;

	// すでに登録済みなら二重登録しないようにチェック
	if (IsTargeting(group)) return;

	// 配列に追加
	targetGroups_.push_back(group);
}

// 2. 指定されたグループをターゲットから外す
void ParticleEmitter::UntargetGroup(ParticleGroup* group) {
	if (group == nullptr) return;

	// ループ（イテレータ）を回して、一致するポインタを探して削除する
	for (auto it = targetGroups_.begin(); it != targetGroups_.end(); ) {
		if (*it == group) {
			it = targetGroups_.erase(it); // 見つけたら削除して次のイテレータを受け取る
		} else {
			++it; // 見つからなければ次に進む
		}
	}
}

// 3. 指定されたグループがすでにターゲットに含まれているか調べる
bool ParticleEmitter::IsTargeting(ParticleGroup* group) const {
	if (group == nullptr) return false;

	// 配列を愚直にループして、同じアドレスのポインタがあるか探す
	for (size_t i = 0; i < targetGroups_.size(); ++i) {
		if (targetGroups_[i] == group) {
			return true; // 見つかった
		}
	}

	return false; // 見つからなかった
}