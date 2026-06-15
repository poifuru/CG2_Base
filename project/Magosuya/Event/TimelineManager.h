#pragma once
#include <vector>
#include <memory>
#include "TimelineEvent.h"
#include "GameObject.h"

class TimelineManager {
public:
	static TimelineManager* GetInstace() {
		static TimelineManager instance;
		return &instance;
	}

	void Update();
	void Draw();

	// エディタ用機能
	void SetCurrentTime(float time);
	float GetCurrentTime() const { return currentTime_; }

	// イベント編集用
	std::vector<TimelineEvent>& GetEvents() { return events_; }
	void AddEvent(const TimelineEvent& event);
	void RemoveEvent(int index);

	// シーン保存・読み込み
	void SaveToFile(const std::string& filePath);
	void LoadFromFile(const std::string& filePath);

private:
	TimelineManager() = default;

	float currentTime_ = 0.0f;
	bool isPlaying_ = false;

	std::vector<TimelineEvent> events_;	// 全てのイベントデータ

	// 現在アクティブなオブジェクトたち
	std::vector<std::unique_ptr<GameObject>> activeObjects_;
};