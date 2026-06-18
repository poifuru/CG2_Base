//#include "Bullet.h"
//#include "Deltatime.h"
//#include "imgui.h"
//#include "Mesh.h"
//#include "MathFunction.h"
//#include "../Enemy/BaseEnemy.h"
//#include "Player.h"
//
//Bullet::Bullet(DxCommon* dxCommon, CameraOrganizer* camera, InputManager* input, LightManager* light) {
//	camera_ = camera;
//	model_ = std::make_unique<Model>(dxCommon, light);
//}
//
//Bullet::~Bullet() {
//
//}
//
//void Bullet::Initialize() {
//	model_->SetModelData("monsterBall.obj");
//	model_->SetTexture("bullet");
//	model_->Initialize();
//
//	aabbSize_ = { 1.0f, 1.0f, 1.0f };
//
//	speed_ = 30.0f;
//	isActive_ = true;
//	activeTimer_ = 3.0f;	// 秒単位
//
//	hitbox_ = true;
//	hitboxColor_ = { 1.0f, 1.0f, 1.0f, 1.0f };
//}
//
//void Bullet::Update() {
//	Move();
//	Life();
//
//	model_->SetPosition(transform_.translate);
//	model_->Update(&camera_->GetCameraData());
//	// AABBの更新
//	aabb_.min = transform_.translate - aabbSize_;
//	aabb_.max = transform_.translate + aabbSize_;
//}
//
//void Bullet::Draw() {
//	model_->Draw();
//
//	if(hitbox_) {
//		DrawHitbox(hitboxColor_);
//	}
//}
//
//void Bullet::ImGui() {
//	model_->ImGui("Bullet");
//	ImGui::Checkbox("hitbox", &hitbox_);
//}
//
//void Bullet::Move() {
//	// ターゲット追従
//	if (target_ && target_->IsActive()) {
//		Vector3 targetPos = target_->GetTransform().translate;
//		
//		bool targetPassed = false;
//
//		// 1. ターゲットがプレイヤーを通り過ぎたか（手前側に来たか）
//		if (camera_ && player_) {
//			Vector3 cameraPos = camera_->GetCameraData().transform.translate;
//			Matrix4x4 camWorld = camera_->GetCameraData().world;
//			Vector3 cameraForward = { camWorld.m[2][0], camWorld.m[2][1], camWorld.m[2][2] };
//
//			float playerDist = Math::Dot(Math::Subtract(player_->GetTransform().translate, cameraPos), cameraForward);
//			float targetDist = Math::Dot(Math::Subtract(targetPos, cameraPos), cameraForward);
//
//			if (targetDist < playerDist) {
//				targetPassed = true;
//			}
//		}
//
//		// 2. ターゲットが弾自身を追い越した（弾より後ろに回り込んだ）か
//		Vector3 toTarget = Math::Subtract(targetPos, transform_.translate);
//		if (Math::Dot(toTarget, direction_) < 0.0f) {
//			targetPassed = true;
//		}
//
//		if (!targetPassed && Math::Length(toTarget) > 0.001f) {
//			toTarget = Math::Normalize(toTarget);
//			
//			// 進行方向を徐々にターゲットの方向へ補間する
//			direction_ = direction_ + (toTarget - direction_) * (homingStrength_ * kDeltaTime);
//			direction_ = Math::Normalize(direction_);
//		} else {
//			target_ = nullptr; // ターゲットが通り過ぎたら誘導解除
//		}
//	} else {
//		target_ = nullptr; // ターゲットが消えたら誘導解除
//	}
//
//	// 進む向きを調整
//	velocity_ = direction_ * speed_;
//
//	if(isActive_) {
//		transform_.translate += velocity_ * kDeltaTime;
//	}
//}
//
//void Bullet::Life() {
//	// 寿命が過ぎたらフラグを折る
//	if(isActive_) { activeTimer_ -= kDeltaTime; }
//	if(activeTimer_ < 0.0f) { isActive_ = false; }
//}
