#include "TimelineManager.h"
#include "Deltatime.h"
#include "GameObjectFactory.h"
#include <fstream>

void TimelineManager::Update() {
	// 再生中なら
	if(isPlaying_) {
		currentTime_ += kDeltaTime;

		// 新しく出現時間(triggerTime)に達したオブジェクトを生成する
		for(const auto& ev : events_) {
			// 現在時間に到達し、かつまだ生成されていない場合
			bool alreadySpawned = false;
			for(const auto& active : activeObjects_) {
				if(active.triggerTime == ev.triggerTime && active.obj->GetTypeName() == ev.objectType) {
					alreadySpawned = true;
					break;
				}
			}

			// currentTimeの直前に出現タイミングがあり、未生成なら生成
			if(!alreadySpawned &&
			   ev.triggerTime <= currentTime_ &&
			   ev.triggerTime > currentTime_ - kDeltaTime) {
				auto newObj = GameObjectFactory::Create(ev.objectType, dx_, light_, input_, camera_);
				if(newObj) {
					// 相対座標をトランスフォームに設定(カメラ一党と合成して配置)
					newObj->SetTranslate(ev.spawnLocalPosition);
					// カスタムパラメータのロード
					activeObjects_.push_back({ std::move(newObj), ev.triggerTime });
				}
			}
		}

		// アクティブなオブジェクトの更新
		for(auto it = activeObjects_.begin(); it != activeObjects_.end();) {
			it->obj->Update();

			// 一定時間経過した、または画面外に出たら削除するなどのライフサイクル管理
			
		}
	}
}

void TimelineManager::Draw() {
	for(auto& active : activeObjects_) {
		active.obj->Draw();
	}
}

void TimelineManager::SetCurrentTime(float time) {
	currentTime_ = time;
	// 時間が飛んだ時、現在アクティブであるべきオブジェクトを再構築する
	RebuildActiveObjects();
}

void TimelineManager::SaveToFile(const std::string& filePath) {
	nlohmann::json root = nlohmann::json::array();
	for(const auto& ev : events_) {
		nlohmann::json evJson;
		ev.Save(evJson);
		root.push_back(evJson);
	}

	std::ofstream file(filePath);
	if(file.is_open()) {
		file << root.dump(4);	// インデント4スペースで保存
	}
}

void TimelineManager::LoadFromFile(const std::string& filePath) {
	std::ifstream file(filePath);
	if(!file.is_open()) return;

	nlohmann::json root;
	file >> root;

	events_.clear();
	activeObjects_.clear();
	currentTime_ = 0.0f;

	for(const auto& evJson : root) {
		TimelineEvent ev;
		ev.Load(evJson);
		events_.push_back(ev);
	}
}

void TimelineManager::RebuildActiveObjects() {
	activeObjects_.clear();

	// 現在時間の時点で出現している且つ消滅していないオブジェクトを再生成
	for(const auto& ev : events_) {
		// 出現時間を過ぎている、且つ出現から5秒以内の場合
		if(ev.triggerTime <= currentTime_ && currentTime_ - ev.triggerTime < 5.0f) {
			auto newObj = GameObjectFactory::Create(ev.objectType, dx_, light_, input_, camera_);
			if(newObj) {
				newObj->SetTranslate(ev.spawnLocalPosition);
				newObj->Load(ev.customParams);
				activeObjects_.push_back({ std::move(newObj), ev.triggerTime });
			}
		}
	}
}