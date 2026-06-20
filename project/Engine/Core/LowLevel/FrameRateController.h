#pragma once
#include <chrono>

class FrameRateController {
public:
	FrameRateController();
	~FrameRateController() = default;

	/// <summary>
	/// フレームの開始時に呼び出して、経過時間を計算する
	/// </summary>
	void Update();

	// --- アクセッサ --- //
	float GetDeltaTime() const { return deltaTime_; }
	float GetFrameRate() const { return frameRate_; }

public:
	// コピー・移動禁止
	FrameRateController(const FrameRateController&) = delete;
	FrameRateController& operator=(const FrameRateController&) = delete;
	FrameRateController(FrameRateController&&) = delete;
	FrameRateController& operator=(FrameRateController&&) = delete;

private:
	// 精密な時間計測のためのタイムスタンプ型
	std::chrono::steady_clock::time_point lastTime_;

	float deltaTime_ = 0.0f; // 前フレームからの経過時間（秒）
	float frameRate_ = 0.0f; // 現在のFPS
};