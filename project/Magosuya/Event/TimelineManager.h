#pragma once
#include <vector>
#include <memory>
#include <string>
#include "TimelineEvent.h"
#include "GameObject.h"

class DxCommon;
class LightManager;
class InputManager;
class CameraOrganizer;

class TimelineManager {
public:
	static TimelineManager* GetInstace() {
		static TimelineManager instance;
		return &instance;
	}

	void Initialize(DxCommon* dx, LightManager* light, InputManager* input, CameraOrganizer* camera) {
		dx_ = dx;
		light_ = light;
		input_ = input;
		camera_ = camera;
	}

	void Update();
	void Draw();

	// タイムラインの時間を直接設定する(シークバー用)
	void SetCurrentTime(float time);
	float GetCurrentTime() const { return currentTime_; }

	// 再生・一時停止の切り替え
	void SetPlaying(bool play) { isPlaying_ = play; }
	bool IsPlayint() const { return isPlaying_; }

	// イベント編集用
	std::vector<TimelineEvent>& GetEvents() { return events_; }
	void AddEvent(const TimelineEvent& event) { events_.push_back(event); }
	void RemoveEvent(int index) { if(index < events_.size()) events_.erase(events_.begin() + index); }

	// シーン保存・読み込み
	void SaveToFile(const std::string& filePath);
	void LoadFromFile(const std::string& filePath);

private:
	TimelineManager() = default;
	TimelineManager(const TimelineManager&) = delete;
	TimelineManager& operator=(const TimelineManager&) = delete;
	// 現在時間において、アクティブになるべきオブジェクトを再構築する
	void RebuildActiveObjects();

private:
	float currentTime_ = 0.0f;
	bool isPlaying_ = false;

	std::vector<TimelineEvent> events_;	// 全てのイベントデータ

	// 現在アクティブなオブジェクトたち
	struct ActiveObject {
		std::unique_ptr<GameObject> obj;
		float triggerTime;	// 出現した時間
	};
	std::vector<ActiveObject> activeObjects_;

	// ポインタ登録
	DxCommon* dx_ = nullptr;
	LightManager* light_ = nullptr;
	InputManager* input_ = nullptr;
	CameraOrganizer* camera_ = nullptr;
};