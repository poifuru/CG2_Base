#include "PCH.h"
#include "BulletComponent.h"
#include "GameObject.h"
#include "DeltaTime.h"
#include "MathFunction.h"

void BulletComponent::Initialize() {
	speed_ = 30.0f;
	activeTimer_ = 3.0f;
}

void BulletComponent::Update() {
	if (!gameObject_) return;

	// ターゲットが生きていれば、その方向へ徐々に弾の向きを曲げる！
	if (target_ && !target_->IsDead()) {
		Vector3 targetPos = target_->GetTransform().translate;
		Vector3 myPos = gameObject_->GetTransform().translate;
		Vector3 toTarget = Math::Subtract(targetPos, myPos);

		if (Math::Length(toTarget) > 0.001f) {
			toTarget = Math::Normalize(toTarget);

			// 旋回力（0.1f = 毎フレーム10%ターゲットの方向へ曲がる）
			float turnSpeed = 0.1f;
			direction_ = Math::Lerp(direction_, toTarget, turnSpeed);
			direction_ = Math::Normalize(direction_);
		}
	}

	// 移動処理
	auto& trans = gameObject_->GetTransform();
	trans.translate = Math::Add(trans.translate, Math::Multiply(speed_ * kDeltaTime, direction_));

	// 弾の回転も進行方向に向ける
	Vector3 bulletRot = { 0.0f, 0.0f, 0.0f };
	if (Math::Length(direction_) > 0.001f) {
		bulletRot.y = std::atan2(direction_.x, direction_.z);
		float xzLength = std::sqrt(direction_.x * direction_.x + direction_.z * direction_.z);
		bulletRot.x = std::atan2(-direction_.y, xzLength);
	}
	trans.rotate = bulletRot;

	// 寿命チェック
	activeTimer_ -= kDeltaTime;
	if (activeTimer_ <= 0.0f) {
		gameObject_->Destroy(); // 親のオブジェクトを破壊（デスフラグを立てる）
	}
}