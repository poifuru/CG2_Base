#include "Boss.h"
#include <algorithm>
#include <numbers>
#include "imgui.h"
#include "InputManager.h"
#include "PlayScene.h"
#include "ModelManager.h"

Boss::Boss(DxCommon* dxCommon, CameraOrganizer* camera) {
	model_ = std::make_unique<Model>(dxCommon);

	ModelManager::GetInstance()->LoadModelData("Resources/boss", "boss");

	camera_ = camera;
}

Boss::~Boss() {

}

void Boss::Initialize() {
	model_->SetModelData("boss");
	model_->SetTexture("boss");
	model_->Initialize();
	model_->IsLighting(LightReflectionModel::HalfLambert);

	transform_.scale = { 2.0f, 2.0f, 2.0f };
	transform_.rotate = { -std::numbers::pi_v<float> / 2.0f, 0.0f, std::numbers::pi_v<float> };
	transform_.translate = { 10.0f, kInitialHeight, 0.0f };
	hp_ = 20;
	isHit_ = false;

	// 初期状態の設定
	state_ = BossState::Wander;
	stateTimer_ = 0.0f;
}

void Boss::Update() {
	stateTimer_ += 1.0f / 60.0f;

	switch(state_) {
	case BossState::Wander:
		WanderBehavior();
		if(stateTimer_ > 5.0f) { state_ = BossState::Dash; stateTimer_ = 0.0f; }
		break;

	case BossState::Dash:
		// 追跡ロジック（target_を使用）
		toPlayer = { target_.translate.x - transform_.translate.x, target_.translate.y - transform_.translate.y, 0.0f };
		length = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
		if(length > 0.1f) { dashVelocity_ = { (toPlayer.x / length) * 0.35f, (toPlayer.y / length) * 0.35f, 0.0f }; }
		DashBehavior();
		if(stateTimer_ > 2.0f) { state_ = BossState::Return; stateTimer_ = 0.0f; }
		break;

	case BossState::Return:
		// 線形補間（Lerp）で y=12.0f に戻る
	{
		float t = std::min<float>(stateTimer_ / 1.0f, 1.0f);
		transform_.translate.y = transform_.translate.y + t * (12.0f - transform_.translate.y);
		if(t >= 1.0f) { state_ = BossState::Shoot; stateTimer_ = 0.0f; }
	}
	break;

	case BossState::Shoot:
		ShootBehavior();
		if(stateTimer_ > 3.0f) { state_ = BossState::Wander; stateTimer_ = 0.0f; }
		break;
	}

	// 移動制限
	transform_.translate.x = std::clamp(transform_.translate.x, -kMaxPosX, kMaxPosX);
	if(transform_.translate.y < 2.0f) transform_.translate.y = 2.0f;

	// 被弾点滅処理
	if(damageTimer_ > 0.0f) {
		damageTimer_ -= 1.0f / 60.0f;
		float alpha = (std::sin(damageTimer_ * 40.0f) > 0.0f) ? 1.0f : 0.3f;
		model_->SetColor({ 1.0f, 0.5f, 0.5f, alpha });
	}
	else {
		model_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	}

	model_->SetTransform(transform_);
	model_->Update(camera_->GetPosition("Debug"), &camera_->GetVPMatrix());
	AABBPos();

	//model_->ImGui("boss");
	//ImGui::Text("hp : %d", hp_);
}

// 1. ゆらゆらさまよう
void Boss::WanderBehavior() {
	floatingTime_ += 0.05f;

	// 前回の 0.02f / 0.01f から大幅にアップしたでやんす！
	// sin(時間) * 大きさ
	transform_.translate.y += std::sin(floatingTime_) * 0.08f;      // 上下の振幅アップ
	transform_.translate.x += std::cos(floatingTime_ * 0.5f) * 0.1f; // 左右の振幅アップ
}

// 2. 突進してくる
void Boss::DashBehavior() {
	// 加速感を出して突っ込むでやんす！
	transform_.translate.x += dashVelocity_.x;
	transform_.translate.y += dashVelocity_.y;
}

// 3. 弾を撃つ
void Boss::ShootBehavior() {
	// --- 修正：回転処理を完全に削除したでやんす ---
	behaviorTimer_ += 1.0f;
}

void Boss::Draw() {
	model_->Draw();
}

void Boss::IsHit(bool attackIsHit) {
	if(attackIsHit && damageTimer_ <= 0.0f) {
		hp_ -= 1;
		damageTimer_ = 0.5f; // 0.5秒点滅
	}
}

void Boss::AABBPos() {
	// ボスの判定もモデルの見た目に合わせて微調整（3.0f -> 2.5fくらいが丁度いいかもでやんす）
	aabb_.min = { transform_.translate.x - 2.5f,  transform_.translate.y - 2.5f, -1.0f };
	aabb_.max = { transform_.translate.x + 2.5f,  transform_.translate.y + 2.5f,  1.0f };
}