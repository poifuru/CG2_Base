#include "Player.h"
#include <algorithm>
#include <numbers>
#include <imgui.h>
#include "InputManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "PlayScene.h"
#include "MathFunction.h"

//プレイヤーの大きさ(半径)
const float kPlayerWidth = 0.9f;
const float kPlayerHeight = 0.9f;

//デルタタイム
const float deltaTime = 1.0f / 60.0f;

//重力
const float kGravity = -0.01f;
// 落下速度の最大値（際限なく加速しないように）
const float kMaxFallSpeed = -0.5f;

Player::Player(DxCommon* dxCommon, CameraOrganizer* camera, InputManager* input, LightManager* light, MapChip* mapchip) {
	isDoubleJump_ = false;
	transform_ = {};
	
	camera_ = camera;
	input_ = input;
	mapchip_ = mapchip;
	model_ = std::make_unique<Model>(dxCommon, light);
	weapon_ = std::make_unique<Weapon>(dxCommon, light);
	for(uint32_t i = 0; i < 3; ++i) {
		life_[i] = std::make_unique<Sprite>(dxCommon);
	}
}

Player::~Player() {

}

void Player::Initialize() {
	model_->Initialize();
	model_->SetModelData("player.obj");
	model_->SetTexture("player");
	SetAABBSize({ kPlayerWidth, kPlayerHeight, 1.0f });

	weapon_->Initialize();
	transform_.translate = mapchip_->GetMapChipPositionByIndex(2, 19);

	for(uint32_t i = 0; i < 3; ++i) {
		life_[i]->SetTexture("life");
		life_[i]->Initialize({});
	}

	hp_ = 3;
	lastHp_ = hp_;
}

void Player::Update() {
	//プレイヤーの挙動をここに
	ProcessDash();
	Input();
	FreeFall();
	WallKickTimer();

	//武器の更新処理
	weapon_->Update(transform_.translate, faceDirection_, updownDirection_, isGrounded_, &camera_->GetCameraData());
	
	CheckMapCollision(mapchip_);
	EntityState();

	// ★ダメージを受けている間（タイマーが動いている間）は紫にする
	if(damageTimer_ > 0) {
		model_->SetColor({ 1.0f, 0.0f, 1.0f, 0.5f });
	}
	else {
		model_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	}

	model_->SetPosition(transform_.translate);
	model_->Update(&camera_->GetCameraData());

	for(uint32_t i = 0; i < 3; ++i) {
		life_[i]->Update();
	}
}

void Player::Draw() {
	model_->Draw();
	weapon_->Draw();

	// ★HPの表示（画面左上などに並べる）
	float startX = 20.0f;  // 開始X座標
	float startY = 20.0f;  // 開始Y座標
	float margin = 60.0f;  // スプライト間の間隔

	for(int i = 0; i < hp_; ++i) {
		Vector3 pos = { startX + (i * margin), startY, 0.0f };

		// シェイクタイマーが動いている間、一番右（ダメージを受けた位置）のライフを震わせる
		if(shakeTimer_ > 0 && i == hp_ - 1) {
			float shakeIntensity = 5.0f; // 震えの強さ
			pos.x += (float)(rand() % 11 - 5) * 0.1f * shakeIntensity;
			pos.y += (float)(rand() % 11 - 5) * 0.1f * shakeIntensity;
		}

		life_[i]->SetPosition(pos);
		life_[i]->Draw();
	}
}

void Player::Input() {
	//加速度の強さ
	const float kAccelerationPower = 0.04f;
	//スピード
	float speed_ = 0.15f;

	//ジャンプ
	float kJumpPower = 0.3f;
	const float kKickPushPower = 0.25f; //壁から離れる方向への力

	//上下の向き
	updownDirection_ = 0.0f;
	if(input_->GetRawInput()->Push('W')) updownDirection_ = 1.0f;
	if(input_->GetRawInput()->Push('S')) updownDirection_ = -1.0f;

	if(!isDashing_) {
		//左右移動
		if(wallKickTimer_ <= 0) {
			velocity_.x = 0.0f;
			if(input_->GetRawInput()->Push('A')) {
				velocity_.x = -speed_;
				faceDirection_ = -1.0f; // 左を向く
			}
			if(input_->GetRawInput()->Push('D')) {
				velocity_.x = speed_;
				faceDirection_ = 1.0f;  // 右を向く
			}
		}

		if(isGrounded_ && input_->GetRawInput()->Trigger(VK_SPACE)) {
			velocity_.y = kJumpPower;
		}
		//壁キック（空中且つ壁に触れている）
		else if(!isGrounded_ && input_->GetRawInput()->Trigger(VK_SPACE)) {
			//右の壁に触れていて、かつ右キーDを押している
			if(isTouchingWallRight_ && input_->GetRawInput()->Push('D')) {
				velocity_.y = kJumpPower;
				velocity_.x = -kKickPushPower; //左に蹴り出す
				isDoubleJump_ = false;
				wallKickTimer_ = 10;           //入力禁止
			}
			//左の壁に触れていて、かつ左キーAを押している
			else if(isTouchingWallLeft_ && input_->GetRawInput()->Push('A')) {
				velocity_.y = kJumpPower;
				velocity_.x = kKickPushPower;  //右に蹴り出す
				isDoubleJump_ = false;
				wallKickTimer_ = 10;           //入力禁止
			}
			isDashing_ = false;
		}
		//二段ジャンプ
		if(!isTouchingWallLeft_ && !isTouchingWallRight_ && !isGrounded_ && !isDoubleJump_ && input_->GetRawInput()->Trigger(VK_SPACE)) {
			velocity_.y = kJumpPower * 0.8f;
			isDoubleJump_ = true;
		}
	}

	if(input_->GetRawInput()->TriggerMouse(0)) {
		weapon_->Attack(faceDirection_, updownDirection_);
	}
}

void Player::FreeFall() {
	if(!isDashing_) {
		//壁ずり判定
		bool isWallSliding = false;
		const float kWallSlideSpeed = -0.15f;

		//右壁に触れていて、Dキー右を押している
		if(isTouchingWallRight_ && input_->GetRawInput()->Push('D')) {
			isWallSliding = true;
		}
		//左壁に触れていて、Aキー左を押している
		else if(isTouchingWallLeft_ && input_->GetRawInput()->Push('A')) {
			isWallSliding = true;
		}

		//重力の適用
		velocity_.y += kGravity;

		//壁ずり中の速度制限
		if(isWallSliding && velocity_.y < kWallSlideSpeed) {
			velocity_.y = kWallSlideSpeed;
		}
	}
}

void Player::WallKickTimer() {
	// タイマーを減らす
	if(wallKickTimer_ > 0) {
		wallKickTimer_--;
	}
}

void Player::ProcessDash() {
	// クールタイムのカウントダウン
	if(dashCooldown_ > 0) {
		dashCooldown_--;
	}

	// ダッシュの開始判定 (ダッシュ中ではなく、クールタイムが終わっているとき)
	if(!isDashing_ && dashCooldown_ <= 0) {
		if(input_->GetRawInput()->TriggerMouse(1)) {
			isDashing_ = true;
			dashTimer_ = 7;    // ダッシュの持続フレーム数
			dashCooldown_ = 30; // 次に出せるまでの時間

			// 向いている方向にダッシュ（A/D入力があればそれを優先）
			if(input_->GetRawInput()->Push('A')) {
				dashDirection_ = -1.0f;
			}
			else if(input_->GetRawInput()->Push('D')) {
				dashDirection_ = 1.0f;
			}
			else {
				// 入力がない場合は現在の速度の向きを参照
				dashDirection_ = (velocity_.x >= 0.0f) ? 1.0f : -1.0f;
			}
		}
	}

	// ダッシュ実行中の処理
	if(isDashing_) {
		const float kDashSpeed = 0.8f; // ダッシュの速さ

		velocity_.x = dashDirection_ * kDashSpeed;
		velocity_.y = 0.0f; // ダッシュ中は重力無視

		dashTimer_--;
		if(dashTimer_ <= 0) {
			isDashing_ = false;
			// 終わった瞬間に慣性を少し残すとスムーズでやんす
			velocity_.x *= 0.5f;
		}
	}
}

void Player::EntityState() {
	if(isGrounded_) {
		isDoubleJump_ = false;
	}

	//ダメージ床の処理
	if(isOnDamageFloor_) {
		if(damageTimer_ <= 0) {
			// ここでHPを減らす（例：hp_ -= 10;）
			// 演出として少し上に跳ねさせるのもアリでやんす！
			hp_--;
			velocity_.y = 0.1f;
			isGrounded_ = false;

			damageTimer_ = 30; // 0.5秒間は無敵など
		}
	}

	if(damageTimer_ > 0) {
		isOnDamageFloor_ = false;
		damageTimer_--;
	}

	// HPが減少した瞬間にシェイク開始
	if(hp_ < lastHp_) {
		shakeTimer_ = 20; // 20フレーム分震えさせる
		lastHp_ = hp_;
	}

	// シェイクタイマーの更新
	if(shakeTimer_ > 0) {
		shakeTimer_--;
	}
}

void Player::ImGui() {
#ifdef USEIMGUI
	model_->ImGui("player");
	ImGui::Text("isGrounded : %d", isGrounded_);
	ImGui::Text("isDoubleJump : %d", isDoubleJump_);
	ImGui::Text("isWallTouchLeft : %d", isTouchingWallLeft_);
	ImGui::Text("isWallTouchRight : %d", isTouchingWallRight_);
	ImGui::Text("isDashing : %d", isDashing_);
	ImGui::Text("hp_ : %d", hp_);

	ImGui::Separator();
	ImGui::Text("--- Debug Collision ---");
	ImGui::Text("AABB Min: (%.3f, %.3f)", aabb_.min.x, aabb_.min.y);
	ImGui::Text("AABB Max: (%.3f, %.3f)", aabb_.max.x, aabb_.max.y);

	// Y軸判定（床判定）で使っているインデックスを再現して表示
	const float kEpsilon = 0.001f;
	IndexSet debugMin = mapchip_->GetMapChipIndexSetByPosition({ aabb_.min.x + kEpsilon, aabb_.min.y + kEpsilon, 0.0f });
	IndexSet debugMax = mapchip_->GetMapChipIndexSetByPosition({ aabb_.max.x - kEpsilon, aabb_.max.y - kEpsilon, 0.0f });

	ImGui::Text("Loop Range X: %u to %u", debugMin.xIndex, debugMax.xIndex);
	ImGui::Text("On Damage Floor: %s", isOnDamageFloor_ ? "TRUE" : "FALSE");
	weapon_->ImGui();
#endif
}