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
#include "../Enemy/EnemyManager.h"
#include "../Enemy/BaseEnemy.h"

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

	ModelManager::GetInstance()->LoadModelData("Resources/AnimatedCube", "AnimatedCube.gltf");
	ModelManager::GetInstance()->LoadAnimationData("Resources/AnimatedCube", "AnimatedCube.gltf");
	TextureManager::GetInstance()->TextureManager::LoadTexture(
		"Resources/AnimatedCube/AnimatedCube_BaseColor.png", "Cube"
	);

	// Bullet用
	TextureManager::GetInstance()->LoadTexture("Resources/bullet/bullet.png", "bullet");
	ModelManager::GetInstance()->LoadModelData("Resources/bullet", "bullet.obj");

	// Reticle用
	TextureManager::GetInstance()->LoadTexture("Resources/reticle/reticle.png", "reticle");
	ModelManager::GetInstance()->LoadModelData("Resources/reticle", "reticle.obj");

	model_ = std::make_unique<Model>(dxCommon, light);
	reticle_ = std::make_unique<Reticle>(dxCommon, camera, input, light);
}

Player::~Player() {

}

void Player::Initialize() {
	model_->SetModelData("player.obj");
	model_->SetTexture("Dummy");
	//model_->SetAnimation("AnimatedCube.gltf");
	model_->Initialize();

	// 固有の数値
	speed_ = 1.5f;
	velocity_ = { 0.0f, 0.0f, 0.0f };
	cooltime_ = 0.0f;
	localTranslate_ = { 0.0f, 0.0f, 0.0f };
	lockedEnemy_ = nullptr;

	// TPS水中化: 初期位置を水中（Y=-10.0f）に設定
	transform_.translate = { 0.0f, -10.0f, 0.0f };

	reticle_->Initialize();
}

void Player::Update() {
	if(camera_->GetActiveCameraName() != "main2") {
		model_->SetAlpha(0.0f);
		return;
	}
	else {
		model_->SetAlpha(1.0f);
	}

	//プレイヤーの挙動をここに
	Input();
	CooltimeUpdate();
	Move();
	BulletsUpdate();

	// 先にレティクルのパラメータを設定し、Updateをかけて最新のワールド座標を計算する
	reticle_->SetPlayerPos(transform_.translate);
	reticle_->SetPlayerLocalPos(localTranslate_);
	reticle_->SetRail(railPath_);
	reticle_->Update();

	// --- ロックオン対象の検索 (スクリーン座標2D判定方式) ---
	lockedEnemy_ = nullptr;
	if (enemyManager_ && camera_) {
		Matrix4x4 vpMat = camera_->GetVPMatrix();
		Vector3 reticlePos = reticle_->GetPosition();
		
		// レティクルをスクリーン座標に変換 (1280x720)
		Vector3 reticleNdc = Math::ChangeTransform(reticlePos, vpMat);
		Vector2 reticleScreen = {
			(reticleNdc.x + 1.0f) * 0.5f * 1280.0f,
			(1.0f - reticleNdc.y) * 0.5f * 720.0f
		};
		
		float minDistance = 999999.0f; // 最もレティクルに近い（スクリーン上で重なっている）敵を優先
		
		for (auto& enemy : enemyManager_->GetEnemies()) {
			if (!enemy || !enemy->IsActive()) continue;
			
			Vector3 enemyPos = enemy->GetTransform().translate;
			
			// --- プレイヤーより手前（カメラ側）に回り込んだ敵は除外する ---
			Vector3 cameraPos = camera_->GetCameraData().transform.translate;
			Matrix4x4 camWorld = camera_->GetCameraData().world;
			Vector3 cameraForward = { camWorld.m[2][0], camWorld.m[2][1], camWorld.m[2][2] };
			Vector3 toEnemy = Math::Subtract(enemyPos, cameraPos);
			
			float playerDist = Math::Dot(Math::Subtract(transform_.translate, cameraPos), cameraForward);
			float enemyDist = Math::Dot(toEnemy, cameraForward);
			if (enemyDist < playerDist) continue;
			
			// スクリーン座標に変換
			Vector3 enemyNdc = Math::ChangeTransform(enemyPos, vpMat);
			Vector2 enemyScreen = {
				(enemyNdc.x + 1.0f) * 0.5f * 1280.0f,
				(1.0f - enemyNdc.y) * 0.5f * 720.0f
			};
			
			float dx = enemyScreen.x - reticleScreen.x;
			float dy = enemyScreen.y - reticleScreen.y;
			float dist = std::sqrt(dx * dx + dy * dy);
			
			if (dist < lockRadius_) {
				if (dist < minDistance) {
					minDistance = dist;
					lockedEnemy_ = enemy.get();
				}
			}
		}
	}

	// レティクルへの状態伝達
	if (lockedEnemy_) {
		reticle_->SetLockOn(true);
	} else {
		reticle_->SetLockOn(false);
	}

	// モデルにデータを渡す
	model_->SetPosition(transform_.translate);

	// プレイヤーモデルの向きを設定
	model_->SetRotate(transform_.rotate);

	model_->Update(&camera_->GetCameraData());
}

void Player::Draw() {
	if(camera_->GetActiveCameraName() != "main2") {
		return;
	}

	model_->Draw();
	BulletsDraw();
}

void Player::DrawUI() {
	if(camera_->GetActiveCameraName() != "main2") {
		return;
	}

	reticle_->Draw();
}

void Player::Input() {
	// *** 移動入力 *** //
	// 加速度をリセット
	acceleration_ = { 0.0f, 0.0f, 0.0f };

	// フレーム内の入力を方向として蓄積
	Vector3 moveDir = { 0.0f, 0.0f, 0.0f };

	if (camera_) {
		// カメラのワールド行列から方向ベクトルを取得
		Matrix4x4 camWorld = camera_->GetCameraData().world;
		Vector3 camForward = { camWorld.m[2][0], camWorld.m[2][1], camWorld.m[2][2] };
		Vector3 camRight = { camWorld.m[0][0], camWorld.m[0][1], camWorld.m[0][2] };

		// 前後左右の移動はすべて水平方向（XZ平面）に制限する
		camForward.y = 0.0f;
		camRight.y = 0.0f;

		if (Math::Length(camForward) > 0.001f) camForward = Math::Normalize(camForward);
		if (Math::Length(camRight) > 0.001f) camRight = Math::Normalize(camRight);

		if (input_->GetRawInput()->Push('W')) { moveDir = Math::Add(moveDir, camForward); }
		if (input_->GetRawInput()->Push('S')) { moveDir = Math::Subtract(moveDir, camForward); }
		if (input_->GetRawInput()->Push('A')) { moveDir = Math::Subtract(moveDir, camRight); }
		if (input_->GetRawInput()->Push('D')) { moveDir = Math::Add(moveDir, camRight); }

		// Spaceで上昇、Left Shiftで下降
		if (input_->GetRawInput()->Push(VK_SPACE)) { moveDir.y += 1.0f; }
		if (input_->GetRawInput()->Push(VK_SHIFT)) { moveDir.y -= 1.0f; }
	}

	// 入力があった場合に処理する
	if (Math::Length(moveDir) > 0.0f) {
		moveDir = Math::Normalize(moveDir);

		// 移動方向（XZ）に向き（ヨー回転）を設定する
		transform_.rotate.y = std::atan2(moveDir.x, moveDir.z);

		// 上下の傾き（ピッチ回転）も移動方向に合わせる
		float xzLength = std::sqrt(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
		transform_.rotate.x = std::atan2(-moveDir.y, xzLength);

		// 実際に速度、デルタタイムを掛ける
		acceleration_.x = moveDir.x * speed_ * kDeltaTime;
		acceleration_.y = moveDir.y * speed_ * kDeltaTime;
		acceleration_.z = moveDir.z * speed_ * kDeltaTime;

		velocity_.x += acceleration_.x;
		velocity_.y += acceleration_.y;
		velocity_.z += acceleration_.z;
	}
	// ****** //

	// *** 弾の発射 *** //
	const float kCooltime = 0.25f;	// 連射間隔の制限
	const uint32_t kMaxBulletCount = 5;	// 同時に存在できる弾の最大数

	// マウス左クリック（ボタン0）で射撃
	if(input_->GetRawInput()->TriggerMouse(0) &&
	   bullets_.size() < kMaxBulletCount &&
	   cooltime_ <= 0.0f) {	// マウス左クリックした瞬間
		// 新しい弾を生成&初期化
		std::unique_ptr<Bullet> newBullet = std::make_unique<Bullet>(dxCommon_, camera_, input_, light_);
		newBullet->Initialize();

		// *** レティクルに向けて飛ばす処理 *** //

		// プレイヤーの現在地を取得してtranslateをセット
		Vector3 startPos = transform_.translate;
		newBullet->SetTranslate(startPos);

		// レティクルの現在地を取得
		Vector3 targetPos = reticle_->GetPosition();

		// 方向ベクトルの計算
		Vector3 direction = {
			targetPos.x - startPos.x,
			targetPos.y - startPos.y,
			targetPos.z - startPos.z,
		};

		// 方向ベクトルを正規化する
		float length = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);

		if(length > 0.0f) {
			direction.x /= length;
			direction.y /= length;
			direction.z /= length;
		}
		else {	// プレイヤーとレティクルの座標が同じの場合
			direction = { 0.0f, 0.0f, 1.0f };
		}

		// 求めた数値をBulletに渡す
		newBullet->SetDirection(direction);
		newBullet->SetPlayer(this);

		if (lockedEnemy_) {
			newBullet->SetTarget(lockedEnemy_);
		}

		// ****** //

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
	velocity_.z *= kAttenuationRate;

	// 速度が上限、下限に触れていないかチェック
	const float maxSpeed = 5.0f;
	velocity_.x = std::clamp(velocity_.x, -maxSpeed, maxSpeed);
	velocity_.y = std::clamp(velocity_.y, -maxSpeed, maxSpeed);
	velocity_.z = std::clamp(velocity_.z, -maxSpeed, maxSpeed);

	// 減衰して速度が一定以下になったら0とみなす
	// 速度の最低値
	const float minSpeed = 0.005f;

	if(std::abs(velocity_.x) < minSpeed) {
		velocity_.x = 0.0f;
	}
	if(std::abs(velocity_.y) < minSpeed) {
		velocity_.y = 0.0f;
	}
	if(std::abs(velocity_.z) < minSpeed) {
		velocity_.z = 0.0f;
	}

	// シンプルに移動量をワールド座標に加算する
	transform_.translate.x += velocity_.x;
	transform_.translate.y += velocity_.y;
	transform_.translate.z += velocity_.z;

	// 水面制限 (Y=-1.0f 以下に制限)
	const float kWaterSurfaceY = -1.0f;
	if (transform_.translate.y > kWaterSurfaceY) {
		transform_.translate.y = kWaterSurfaceY;
		velocity_.y = 0.0f;
	}
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
	
	ImGui::Separator();
	ImGui::Text("--- LockOn Debug ---");
	if (lockedEnemy_) {
		ImGui::Text("Locked Enemy: Yes (Address: %p)", lockedEnemy_);
	} else {
		ImGui::Text("Locked Enemy: No");
	}
	
	if (enemyManager_) {
		auto& enemies = enemyManager_->GetEnemies();
		int activeCount = 0;
		for (const auto& e : enemies) {
			if (e && e->IsActive()) activeCount++;
		}
		ImGui::Text("Active Enemies: %d / %d", activeCount, (int)enemies.size());
			
		if (camera_ && reticle_) {
			Matrix4x4 vpMat = camera_->GetVPMatrix();
			Vector3 retPos = reticle_->GetPosition();
			Vector3 retNdc = Math::ChangeTransform(retPos, vpMat);
			ImGui::Text("Reticle World: (%.1f, %.1f, %.1f)", retPos.x, retPos.y, retPos.z);
			ImGui::Text("Reticle NDC: (%.3f, %.3f, %.3f)", retNdc.x, retNdc.y, retNdc.z);
			
			int idx = 0;
			for (auto& enemy : enemies) {
				if (enemy && enemy->IsActive()) {
					Vector3 ePos = enemy->GetTransform().translate;
					Vector3 eNdc = Math::ChangeTransform(ePos, vpMat);
					ImGui::Text("Enemy[%d] World: (%.1f, %.1f, %.1f)", idx, ePos.x, ePos.y, ePos.z);
					ImGui::Text("Enemy[%d] NDC: (%.3f, %.3f, %.3f)", idx, eNdc.x, eNdc.y, eNdc.z);
					idx++;
				}
			}
		}
	}
	ImGui::End();
#endif
}