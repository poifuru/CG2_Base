#include "Player.h"
#include <algorithm>
#include <numbers>
#include <imgui.h>
#include "InputManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "PlayScene.h"
#include "MathFunction.h"
#include "DeltaTime.h"

// キー入力が無いときに速度を減衰させる定数
static const float kAttenuationRate = 0.95f;

Player::Player(DxCommon* dxCommon, CameraOrganizer* camera, InputManager* input, LightManager* light) {
	transform_ = {};

	dxCommon_ = dxCommon;
	camera_ = camera;
	input_ = input;
	light_ = light;

	TextureManager::GetInstance()->LoadTexture("Resources/player/player.png", "player");
	ModelManager::GetInstance()->LoadModelData("Resources/player", "player.obj");

	// Bullet用
	TextureManager::GetInstance()->LoadTexture("Resources/monsterBall/monsterBall.png", "bullet");
	ModelManager::GetInstance()->LoadModelData("Resources/monsterBall", "monsterBall.obj");

	// Reticle用
	TextureManager::GetInstance()->LoadTexture("Resources/reticle/reticle.png", "reticle");
	ModelManager::GetInstance()->LoadModelData("Resources/reticle", "reticle.obj");
}

Player::~Player() {

}

void Player::Initialize() {
	model_ = std::make_unique<Model>(dxCommon_, light_);
	model_->SetModelData("player.obj");
	model_->SetTexture("player");
	model_->Initialize();

	// 固有の数値
	speed_ = 1.5f;
	velocity_ = { 0.0f, 0.0f, 5.0f };
	cooltime_ = 0.0f;

	reticle_ = std::make_unique<Reticle>(dxCommon_, camera_, input_, light_);
	//reticle_->Initialize();
}

void Player::Update() {
	//プレイヤーの挙動をここに
	Input();
	CooltimeUpdate();
	Move();
	BulletsUpdate();
	//reticle_->SetPlayerPos(transform_.translate);
	//reticle_->Update();

	// モデルにデータを渡す
	model_->SetPosition(transform_.translate);
	model_->Update(&camera_->GetCameraData());
}

void Player::Draw() {
	model_->Draw();
	BulletsDraw();
	//reticle_->Draw();
}

void Player::Input() {
	// *** 移動入力 *** //
	// 加速度をリセット
	acceleration_.x = 0.0f;
	acceleration_.y = 0.0f;

	// フレーム内の入力を方向として蓄積
	Vector2 moveDir = { 0.0f, 0.0f };

	if(input_->GetRawInput()->Push('W')) { moveDir.y += 1.0f; }
	if(input_->GetRawInput()->Push('S')) { moveDir.y -= 1.0f; }
	if(input_->GetRawInput()->Push('A')) { moveDir.x -= 1.0f; }
	if(input_->GetRawInput()->Push('D')) { moveDir.x += 1.0f; }

	// 入力があった場合に処理する
	if(moveDir.x != 0.0f || moveDir.y != 0.0f) {
		// ベクトルの長さを計算
		float length = std::sqrt(moveDir.x * moveDir.x + moveDir.y * moveDir.y);

		// 正規化
		moveDir.x /= length;
		moveDir.y /= length;

		// 実際に速度、デルタタイムを掛ける
		acceleration_.x = moveDir.x * speed_ * kDeltaTime;
		acceleration_.y = moveDir.y * speed_ * kDeltaTime;

		velocity_.x += acceleration_.x;
		velocity_.y += acceleration_.y;
	}
	// ****** //

	// *** 弾の発射 *** //
	const float kCooltime = 0.25f;	// 連射間隔の制限
	const uint32_t kMaxBulletCount = 5;	// 同時に存在できる弾の最大数

	if(input_->GetRawInput()->Trigger(VK_SPACE) &&
	   bullets_.size() < kMaxBulletCount &&
	   cooltime_ <= 0.0f) {	// スペースキーを押した瞬間
		// 新しい弾を生成&初期化
		std::unique_ptr<Bullet> newBullet = std::make_unique<Bullet>(dxCommon_, camera_, input_, light_);
		newBullet->Initialize();

		// 位置をプレイヤーに合わせる
		newBullet->SetTranslate(transform_.translate);

		// リストに追加
		bullets_.push_back(std::move(newBullet));

		// クールタイムを設定
		cooltime_ = kCooltime;
	}
	// ****** //
}

void Player::CooltimeUpdate() {
	if(cooltime_ > 0.0f) {
		cooltime_ -= kDeltaTime;
	}
}

void
Player::Move() {
	// 速度に減衰率をかけ続ける
	velocity_.x *= kAttenuationRate;
	velocity_.y *= kAttenuationRate;

	// 速度が上限、下限に触れていないかチェック
	const float maxSpeed = 5.0f;
	velocity_.x = std::clamp(velocity_.x, -maxSpeed, maxSpeed);
	velocity_.y = std::clamp(velocity_.y, -maxSpeed, maxSpeed);

	// 減衰して速度が一定以下になったら0とみなす
	// 速度の最低値
	const float minSpeed = 0.005f;

	if(std::abs(velocity_.x) < minSpeed) {
		velocity_.x = 0.0f;
	}
	if(std::abs(velocity_.y) < minSpeed) {
		velocity_.y = 0.0f;
	}

	// 実際の移動処理
	transform_.translate.x += velocity_.x;
	transform_.translate.y += velocity_.y;
	transform_.translate.z += velocity_.z * kDeltaTime;

	// 移動制限
	transform_.translate.x = std::clamp(transform_.translate.x, -18.0f, 18.0f);
	transform_.translate.y = std::clamp(transform_.translate.y, -10.0f, 10.0f);
}

void Player::BulletsUpdate() {
	// イテレータでループを回す
	for(auto it = bullets_.begin(); it != bullets_.end();) {
		// 更新
		(*it)->Update();

		// 弾の寿命が過ぎていたらリストから削除
		if(!(*it)->IsActive()) {
			it = bullets_.erase(it);
		}
		else {
			// 削除されなかったら次に進む
			++it;
		}
	}
}

void Player::BulletsDraw() {
	for(auto it = bullets_.begin(); it != bullets_.end(); ++it) {
		(*it)->Draw();
	}
}

void Player::ImGui() {
#ifdef USEIMGUI
	ImGui::Begin("Player");
	ImGui::DragFloat3("Position", &transform_.translate.x, 0.01f);
	ImGui::DragFloat3("Velocity", &velocity_.x, 0.01f);
	ImGui::DragFloat3("Acceleration", &acceleration_.x, 0.01f);
	ImGui::End();
#endif
}