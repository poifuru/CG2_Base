#include "PCH.h"
#include "BulletComponent.h"
#include "GameObject.h"
#include "DeltaTime.h"

void BulletComponent::Initialize() {
	speed_ = 30.0f;
	activeTimer_ = 3.0f;
}

void BulletComponent::Update() {
	// 直進移動
	auto& transform = gameObject_->GetTransform();
	transform.translate.x += direction_.x * speed_ * kDeltaTime;
	transform.translate.y += direction_.y * speed_ * kDeltaTime;
	transform.translate.z += direction_.z * speed_ * kDeltaTime;
	// 寿命チェック
	activeTimer_ -= kDeltaTime;
	if (activeTimer_ <= 0.0f) {
		gameObject_->Destroy(); // 親のオブジェクトを破壊（デスフラグを立てる）
	}
}