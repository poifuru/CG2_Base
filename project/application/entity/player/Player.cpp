#include "Player.h"
#include <algorithm>
#include <numbers>
#include <imgui.h>
#include "InputManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "PlayScene.h"
#include "MathFunction.h"

//定数
const inline float deltaTime = 1.0f / 60.0f;
const inline float gravity = -1.0f;

Player::Player(DxCommon* dxCommon, CameraOrganizer* camera) {
	model_ = std::make_unique<Model>(dxCommon);
	weapon_ = std::make_unique<Model>(dxCommon);

	ModelManager::GetInstance()->LoadModelData("Resources/player", "player");
	ModelManager::GetInstance()->LoadModelData("Resources/hammer", "hammer");

	camera_ = camera;
	input_ = InputManager::GetInstance();

	//カメラ追加
	camera_->AddCamera("follow1", CameraType::FollowCamera);
}

Player::~Player() {
	
}

void Player::Initialize() {
	
	model_->SetModelData("player");
	model_->SetTexture("player");
	model_->Initialize();
	model_->IsLighting(LightReflectionModel::HalfLambert);

	weapon_->SetModelData("hammer");
	weapon_->SetTexture("hammer");
	weapon_->Initialize();
	weapon_->IsLighting(LightReflectionModel::HalfLambert);

	transform_ = model_->GetTransform();
	transform_.rotate = { -std::numbers::pi_v<float> / 2.0f, 0.0f, 0.0f };
	transform_.translate = { 0.0f, 5.0f, 0.0f };
	transformWep_ = weapon_->GetTransform();
	transformWep_.rotate = { -std::numbers::pi_v<float> / 2.0f, -std::numbers::pi_v<float> / 2.0f, 0.0f };

	camera_->SetFollowTarget("follow1", transform_);
	camera_->SetActiveCamera("follow1");

	velocity_ = { 0.0f, 0.0f, 0.0f };
	dir_ = Direction::Right;
	hp_ = 3;
	isInvincible_ = false;
	invincibleTime_ = 1.0f;
	isAir_ = false;
	airJump_ = false;
	isAvoid_ = false;
	avoidTimer_ = 0.2f;
	coolTime_ = false;
	avoidCoolTime_ = 0.8f;
	isAttack_ = false;
	attackTime_ = 0.1f;
}

void Player::Update() {
	move();
	Attack();

	model_->SetTransform(transform_);
	model_->Update(camera_->GetPosition("Debug"), &camera_->GetVPMatrix());
	model_->SetColor(Vector4{ 1.0f, 1.0f, 1.0f, 1.0f });
	WeaponTransform();
	weapon_->SetTransform(transformWep_);
	weapon_->Update(camera_->GetPosition("Debug"), &camera_->GetVPMatrix());
	AABBPos();
	Invincible();
}

void Player::Draw() {
	model_->Draw();
	if(isAttack_) {
		weapon_->Draw();
	}
}

void Player::IsHit() {
	if(!isInvincible_) {
		hp_ -= 1;
		isInvincible_ = true;
	}
}

void Player::move() {
	if(input_->GetRawInput()->Push('A') && !isAvoid_) {
		dir_ = Direction::Left;
		velocity_.x = -0.2f;
		TurnControll();
	}
	if(input_->GetRawInput()->Push('D') && !isAvoid_) {
		dir_ = Direction::Right;
		TurnControll();
		velocity_.x = 0.2f;
	}
	if(!input_->GetRawInput()->Push('A') && !input_->GetRawInput()->Push('D')) {
		velocity_.x = 0.0f;
		TurnControll();
	}
	Jump();
	Avoid();

	transform_.translate.x += velocity_.x;
	if(!isAvoid_) {
		velocity_.y += gravity * deltaTime;
	}
	transform_.translate.y += velocity_.y;

	if(transform_.translate.y <= 2.0f) {
		transform_.translate.y = 2.0f;
		isAir_ = false;
		airJump_ = true;
	}
	else {
		isAir_ = true;
	}

	//移動制限
	if(transform_.translate.x <= -38.0f) {
		transform_.translate.x = -38.0f;
	}
	else if(transform_.translate.x >= 38.0f) {
		transform_.translate.x = 38.0f;
	}
	if(transform_.translate.y >= 38.0f) {
		transform_.translate.y = 38.0f;
		velocity_.y = 0.0f;
	}
}

void Player::Jump() {
	if(input_->GetRawInput()->Trigger(VK_SPACE) && !isAir_) {
		velocity_.y = 0.6f;
	}
	if(input_->GetRawInput()->Trigger(VK_SPACE) && isAir_ && airJump_) {
		velocity_.y = 0.4f;
		airJump_ = false;
	}
}

void Player::Avoid() {
	if(input_->GetRawInput()->TriggerMouse(MouseButton::RIGHT) && !isAvoid_ && avoidCoolTime_ == 0.8f) {
		if(dir_ == Direction::Left) {
			velocity_.y = 0.0f;
			velocity_.x = -2.0f;
		}
		else if(dir_ == Direction::Right) {
			velocity_.y = 0.0f;
			velocity_.x = 1.5f;
		}
		isAvoid_ = true;
		coolTime_ = true;
	}
	if(isAvoid_) {
		avoidTimer_ -= deltaTime;
		if(velocity_.x >= 0.0f && dir_ == Direction::Right && avoidTimer_ >= 0.0f) {
			velocity_.x -= 0.1f;
		}
		if(velocity_.x <= 0.0f && dir_ == Direction::Left && avoidTimer_ >= 0.0f) {
			velocity_.x += 0.1f;
		}
		if(avoidTimer_ <= 0.0f) {
			avoidTimer_ = 0.2f;
			isAvoid_ = false;
		}
	}
	if(coolTime_) {
		avoidCoolTime_ -= deltaTime;
	}
	if(avoidCoolTime_ <= 0.0f) {
		avoidCoolTime_ = 0.8f;
		coolTime_ = false;
	}
}

void Player::Attack() {
	if(input_->GetRawInput()->TriggerMouse(MouseButton::LEFT)) {
		isAttack_ = true;
		attackDir_ = dir_;
		// Wキーを押していたら上攻撃フラグを立てる
		isUpperAttack_ = input_->GetRawInput()->Push('W');
	}
	if(isAttack_) {
		attackTime_ -= deltaTime;
	}
	if(attackTime_ <= 0.0f) {
		isAttack_ = false;
		isUpperAttack_ = false; // リセット
		attackTime_ = 0.5f;
		attackIsHit_ = false;
	}
}

void Player::WeaponTransform() {
	Direction currentWeaponDir = isAttack_ ? attackDir_ : dir_;

	if(isUpperAttack_) {
		// 上攻撃：プレイヤーの頭上に配置
		transformWep_.translate.x = transform_.translate.x;
		transformWep_.translate.y = transform_.translate.y + 3.0f;

		// 【重要】左向きの時に回転が逆転するのを防ぐロジック
		// X回転を調整して、ハンマーの頭が上を向くように固定するでやんす
		transformWep_.rotate.x = 0.0f;
		transformWep_.rotate.z = 0.0f;
		transformWep_.rotate.y = (currentWeaponDir == Direction::Left) ? 0.0f : std::numbers::pi_v<float>;
	}
	else {
		// 通常攻撃：左右に配置
		transformWep_.rotate.x = -std::numbers::pi_v<float> / 2.0f;
		transformWep_.translate.y = transform_.translate.y;
		if(currentWeaponDir == Direction::Left) {
			transformWep_.translate.x = transform_.translate.x - 3.0f;
			transformWep_.rotate.y = std::numbers::pi_v<float> / 2.0f;
		}
		else {
			transformWep_.translate.x = transform_.translate.x + 3.0f;
			transformWep_.rotate.y = -std::numbers::pi_v<float> / 2.0f;
		}
	}
}

void Player::AABBPos() {
	// プレイヤーの判定
	aabb_.min = { transform_.translate.x - 1.0f,  transform_.translate.y - 1.0f, 0.0f };
	aabb_.max = { transform_.translate.x + 1.0f,  transform_.translate.y + 1.0f, 0.0f };

	// 【修正】aabb_weapon_ に代入するように修正（aabb_ を上書きしていたでやんす！）
	aabb_weapon_.min = { transformWep_.translate.x - 2.0f,  transformWep_.translate.y - 2.0f, 0.0f };
	aabb_weapon_.max = { transformWep_.translate.x + 2.0f,  transformWep_.translate.y + 2.0f, 0.0f };
}

void Player::Invincible() {
	if(isInvincible_) {
		invincibleTime_ -= deltaTime;
		// 点滅：sin波を使って透明度をチカチカさせる
		float alpha = (std::sin(invincibleTime_ * 40.0f) > 0.0f) ? 1.0f : 0.2f;
		model_->SetColor({ 1.0f, 0.7f, 0.7f, alpha });
	}
	else {
		model_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	}
	if(invincibleTime_ <= 0.0f) {
		isInvincible_ = false;
		invincibleTime_ = 1.0f;
	}
}

void Player::TurnControll() {
	// ===== 旋回処理 =====
	if(turnTimer_ > 0.0f) {
		turnTimer_ -= deltaTime;
		float t = 1.0f - (turnTimer_ / kTimeTurn);
		float destinationRotationYTable[] = {
			std::numbers::pi_v<float> / 2.0f,
			std::numbers::pi_v<float> *3.0f / 2.0f,
		};
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(dir_)];
		transform_.rotate.y = Math::Lerp(turnFirstRotationY_, destinationRotationY, t);
	}	
}
