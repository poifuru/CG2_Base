#include "PCH.h"
#include "FrameRateController.h"

FrameRateController::FrameRateController() {
	// 初期化時点のタイムスタンプを記録しておく
	lastTime_ = std::chrono::steady_clock::now();
}

void FrameRateController::Update() {
	// 現在の時間を取得
	std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();

	// 前回からの経過時間をマイクロ秒単位で計算
	auto elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastTime_);

	// 次のフレームのために現在の時間を保存
	lastTime_ = currentTime;

	// マイクロ秒から「秒」に変換してデルタタイムにする (1秒 = 1,000,000マイクロ秒)
	deltaTime_ = static_cast<float>(elapsedTime.count()) / 1000000.0f;

	// スパイク（急激な処理落ちやデバッグブレークポイントによる停止）対策
	// 1フレームが極端に長い（例: 0.1秒以上）場合は、挙動が破綻しないように上限をキャップする
	if (deltaTime_ > 0.1f) {
		deltaTime_ = 0.1f;
	}

	// 現在のFPSの計算（1秒 / 1フレームの時間）
	if (deltaTime_ > 0.0f) {
		frameRate_ = 1.0f / deltaTime_;
	}
}