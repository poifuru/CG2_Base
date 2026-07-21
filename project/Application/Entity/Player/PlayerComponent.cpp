#include "PCH.h"
#include "PlayerComponent.h"
#include "GameObject.h"
#include "InputManager.h"
#include "RawInput.h"
#include "GamePad.h"
#include "DeltaTime.h"
#include "MathFunction.h"
#include "CameraOrganizer.h"
#include "BaseCamera.h" // CameraDataの定義があるヘッダー
#include "MeshRendererComponent.h"
#include "BulletComponent.h"
#include "BaseScene.h"
#include "ReticleComponent.h"
#include "ColliderComponent.h"
#include "BlendModeManager.h"
#include "../../../../Engine/Editor/ParticleEditor/ParticleSpawner.h"

void PlayerComponent::Initialize() {
	if (isInitialized_) return;
	isInitialized_ = true;

	hp_ = 5;
	maxHp_ = 5;
	invincibilityTimer_ = 0.0f;
	invincibilityDuration_ = 1.5f;
	isDead_ = false;

	speed_ = 0.5f;
	velocity_ = { 0.0f, 0.0f, 0.0f };
	cooltime_ = 0.0f;
	turnSpeed_ = 0.3f;
	dirRatioZ_ = 0.97f;
	dirRatioX_ = 0.03f;
	attenuationRate_ = 0.98f;
	brakeAttenuationRate_ = 0.90f;
	maxSpeed_ = 2.5f; 
	preTriggerR_ = 0.0f;
	brakeTurnSpeedMultiplier_ = 3.0f;
	brakeMoveSpeedMultiplier_ = 0.2f;

	if (gameObject_) {
		gameObject_->GetTransform().translate = { 0.0f, 0.3f, 0.0f };

		// プレイヤーにコライダーがなければ自動追加
		auto* collider = gameObject_->GetComponent<ColliderComponent>();
		if (!collider) {
			collider = gameObject_->AddComponent<ColliderComponent>();
		}
		if (collider) {
			collider->SetRadius(2.0f); // 船のサイズに合わせた球判定
		}
	}
}

void PlayerComponent::Update() {
	if(isDebugMode_) {
		return;
	}

	// クールタイム更新
	if(cooltime_ > 0.0f) {
		cooltime_ -= kDeltaTime;
	}

	// 無敵タイマーと被弾点滅演出
	if (invincibilityTimer_ > 0.0f) {
		invincibilityTimer_ -= kDeltaTime;
		if (invincibilityTimer_ < 0.0f) invincibilityTimer_ = 0.0f;

		// 赤く点滅
		bool flashRed = (static_cast<int>(invincibilityTimer_ * 10.0f) % 2 == 0);
		if (auto* mesh = gameObject_->GetComponent<MeshRendererComponent>()) {
			mesh->SetColor({ 1.0f, flashRed ? 0.3f : 1.0f, flashRed ? 0.3f : 1.0f, 1.0f });
		}
	}
	else {
		if (auto* mesh = gameObject_->GetComponent<MeshRendererComponent>()) {
			mesh->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
		}
	}

	InputManager::GetInstance()->GetGamePad()->SetStickDeadZone(2000);

	// 移動処理
	Move();

	// 射撃処理
	Shoot();

	// レティクルへのハープーン有効射程（ロックオン距離）の自動同期
	if (reticleObject_) {
		if (auto* reticleComp = reticleObject_->GetComponent<ReticleComponent>()) {
			reticleComp->SetLockOnMaxDistance(harpoonMaxDistance_);
		}
	}
}

void PlayerComponent::TakeDamage(int damage) {
	if (isDead_ || invincibilityTimer_ > 0.0f) return;

	hp_ -= damage;
	invincibilityTimer_ = invincibilityDuration_;

	// 被弾エフェクト
	if (gameObject_) {
		ParticleSpawner::SpawnExplosion(gameObject_->GetContext(), gameObject_->GetTransform().translate, 10);
	}

	// カメラシェイク
	CameraOrganizer::GetInstance()->Shake(0.35f, 0.4f);

	if (hp_ <= 0) {
		hp_ = 0;
		isDead_ = true;
	}
}

void PlayerComponent::ResolveReticle(const std::vector<std::unique_ptr<GameObject>>& gameObjects) {
	for (const auto& obj : gameObjects) {
		if (obj->GetComponent<ReticleComponent>()) {
			reticleObject_ = obj.get();
			return;
		}
	}
}

void PlayerComponent::Move() {
	// 起動時に合計が 1.0f からズレていた場合のための自動補正
	dirRatioZ_ = std::clamp(dirRatioZ_, 0.0f, 1.0f);
	dirRatioX_ = 1.0f - dirRatioZ_;

	// 入力用のポインタを取得
	InputManager* input = InputManager::GetInstance();

	// ブレーキ入力の判定
	bool isBrakePressed = input->GetRawInput()->Push(VK_SPACE);
	if (input->GetGamePad()->IsConection()) {
		if (input->GetGamePad()->PushButton(Button::A)) {
			isBrakePressed = true;
		}
	}
	// ブレーキ状態に応じて旋回力と移動力を変動させる
	float currentTurnSpeed = turnSpeed_;
	float currentSpeed = speed_;
	if (isBrakePressed) {
		currentTurnSpeed = turnSpeed_ * brakeTurnSpeedMultiplier_;
		currentSpeed = speed_ * brakeMoveSpeedMultiplier_;
	}

	acceleration_ = { 0.0f, 0.0f, 0.0f };
	Vector3 moveDir = { 0.0f, 0.0f, 0.0f };

	// カメラのワールド行列から方向ベクトルを取得
	Matrix4x4 camWorld = CameraOrganizer::GetInstance()->GetCameraData().world;
	Vector3 camForward = { camWorld.m[2][0], camWorld.m[2][1], camWorld.m[2][2] };
	Vector3 camRight = { camWorld.m[0][0], camWorld.m[0][1], camWorld.m[0][2] };

	// 前後左右の移動はすべて水平方向（XZ平面）に制限
	camForward.y = 0.0f;
	camRight.y = 0.0f;
	if(Math::Length(camForward) > 0.001f) camForward = Math::Normalize(camForward);
	if(Math::Length(camRight) > 0.001f) camRight = Math::Normalize(camRight);

	// キーボード入力で移動方向を蓄積
	if(input->GetRawInput()->Push('W')) { moveDir = Math::Add(moveDir, camForward); }
	if(input->GetRawInput()->Push('S')) { moveDir = Math::Subtract(moveDir, camForward); }
	if(input->GetRawInput()->Push('A')) { moveDir = Math::Subtract(moveDir, camRight); }
	if(input->GetRawInput()->Push('D')) { moveDir = Math::Add(moveDir, camRight); }

	// ゲームパッド入力で移動方向を蓄積
	if (input->GetGamePad()->IsConection()) {
		Vector2 lStick = input->GetGamePad()->GetStick(LR::Left);
		if (lStick.x != 0.0f || lStick.y != 0.0f) {
			moveDir = Math::Add(moveDir, Math::Multiply(lStick.y, camForward));
			moveDir = Math::Add(moveDir, Math::Multiply(lStick.x, camRight));
		}
	}

	// Spaceで上昇、Left Shiftで下降
	/*if(input->GetRawInput()->Push(VK_SPACE)) { moveDir.y += 1.0f; }
	if(input->GetRawInput()->Push(VK_SHIFT)) { moveDir.y -= 1.0f; }*/

	// 入力があった場合に移動と回転を設定する
	if(Math::Length(moveDir) > 0.0f) {
		moveDir = Math::Normalize(moveDir);

		// 向き（Yaw回転 / Pitch回転）を徐々に補間して近づける
		// 左右の旋回目標角度 (Yaw)
		float targetYaw = std::atan2(moveDir.x, moveDir.z);
		float currentYaw = gameObject_->GetTransform().rotate.y;

		// 角度の最短差分を求める
		float diffYaw = targetYaw - currentYaw;
		while (diffYaw < -3.14159265f) diffYaw += 6.2831853f;
		while (diffYaw > 3.14159265f) diffYaw -= 6.2831853f;

		// 旋回速度 (値が小さいほどゆっくり曲がる)
		gameObject_->GetTransform().rotate.y += diffYaw * currentTurnSpeed * kDeltaTime;

		// 上下の旋回目標角度 (Pitch)
		float xzLength = std::sqrt(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
		float targetPitch = std::atan2(-moveDir.y, xzLength);
		float currentPitch = gameObject_->GetTransform().rotate.x;
		float diffPitch = targetPitch - currentPitch;
		while (diffPitch < -3.14159265f) diffPitch += 6.2831853f;
		while (diffPitch > 3.14159265f) diffPitch -= 6.2831853f;
		gameObject_->GetTransform().rotate.x += diffPitch * currentTurnSpeed * kDeltaTime;

		// 移動ベクトルのブレンド (前進 dirRatioZ_ : 入力 dirRatioX_)
		// 潜水艦の「現在の正面方向」のベクトルを計算する
		float cy = std::cos(gameObject_->GetTransform().rotate.y);
		float sy = std::sin(gameObject_->GetTransform().rotate.y);
		float cx = std::cos(gameObject_->GetTransform().rotate.x);
		float sx = std::sin(gameObject_->GetTransform().rotate.x);

		Vector3 submarineForward = { sy * cx, -sx, cy * cx };
		if(Math::Length(submarineForward) > 0.001f) {
			submarineForward = Math::Normalize(submarineForward);
		}

		// 実際の進む方向 = (正面方向 * dirRatioZ_) + (入力された移動方向 * dirRatioX_)
		Vector3 actualMoveDir = {};
		actualMoveDir.x = submarineForward.x * dirRatioZ_ + moveDir.x * dirRatioX_;
		actualMoveDir.y = submarineForward.y * dirRatioZ_ + moveDir.y * dirRatioX_;
		actualMoveDir.z = submarineForward.z * dirRatioZ_ + moveDir.z * dirRatioX_;

		if(Math::Length(actualMoveDir) > 0.001f) {
			actualMoveDir = Math::Normalize(actualMoveDir);
		}

		// 加速度を設定
		acceleration_.x = actualMoveDir.x * currentSpeed * kDeltaTime;
		acceleration_.y = actualMoveDir.y * currentSpeed * kDeltaTime;
		acceleration_.z = actualMoveDir.z * currentSpeed * kDeltaTime;
		velocity_.x += acceleration_.x;
		velocity_.y += acceleration_.y;
		velocity_.z += acceleration_.z;
	}

	// 速度の減衰と制限
	// 通常は attenuationRate_ だが、ブレーキ中は強いブレーキにする
	float currentAttenuation = attenuationRate_;
	
	if (isBrakePressed) {
		currentAttenuation = brakeAttenuationRate_;
	}

	velocity_.x *= currentAttenuation;
	velocity_.y *= currentAttenuation;
	velocity_.z *= currentAttenuation;

	// クランプで最高・最低速度を制限
	velocity_.x = std::clamp(velocity_.x, -maxSpeed_, maxSpeed_);
	velocity_.y = std::clamp(velocity_.y, -maxSpeed_, maxSpeed_);
	velocity_.z = std::clamp(velocity_.z, -maxSpeed_, maxSpeed_);

	const float minSpeed = 0.00001f;
	if(std::abs(velocity_.x) < minSpeed) { velocity_.x = 0.0f; }
	if(std::abs(velocity_.y) < minSpeed) { velocity_.y = 0.0f; }
	if(std::abs(velocity_.z) < minSpeed) { velocity_.z = 0.0f; }

	// 位置の加算
	gameObject_->GetTransform().translate.x += velocity_.x;
	gameObject_->GetTransform().translate.y += velocity_.y;
	gameObject_->GetTransform().translate.z += velocity_.z;

	// 水面制限 (Y=-1.0f 以下)
	/*const float kWaterSurfaceY = -1.0f;
	if(gameObject_->GetTransform().translate.y > kWaterSurfaceY) {
		gameObject_->GetTransform().translate.y = kWaterSurfaceY;
		velocity_.y = 0.0f;
	}*/

	// 移動トレイルの発生
	float speed = Math::Length(velocity_);
	if (speed > 0.05f) {
		// 潜水艦の現在の回転（向き）から、正面ベクトルを計算する
		float ry = gameObject_->GetTransform().rotate.y;
		float rx = gameObject_->GetTransform().rotate.x;
		Vector3 forward = {
			std::sin(ry) * std::cos(rx),
			-std::sin(rx),
			std::cos(ry) * std::cos(rx)
		};
		if (Math::Length(forward) > 0.001f) {
			forward = Math::Normalize(forward);
		}

		// 進行方向ではなく、常に潜水艦の「真後ろ」から出るようにオフセットを計算
		Vector3 offset = Math::Multiply(2.1f, forward);
		Vector3 spawnPos = Math::Subtract(gameObject_->GetTransform().translate, offset);
		spawnPos.y = spawnPos.y - 0.1f; // 高さの微調整

		Vector3 moveDir = Math::Normalize(velocity_);
		ParticleSpawner::SpawnTrail(gameObject_->GetContext(), spawnPos, moveDir);
	}
}

void PlayerComponent::Shoot() {
	InputManager* input = InputManager::GetInstance();

	bool isShootTriggered = input->GetRawInput()->TriggerMouse(0);
	if (input->GetGamePad()->IsConection()) {
		float triggerR = input->GetGamePad()->GetTrigger(LR::Right);
		if (triggerR > 0.5f && preTriggerR_ <= 0.5f) {
			isShootTriggered = true;
		}
		preTriggerR_ = triggerR;
	} else {
		preTriggerR_ = 0.0f;
	}

	if (isShootTriggered && cooltime_ <= 0.0f) {
		auto* context = gameObject_->GetContext();
		if (!context || !context->gameObjects) return;

		// 弾用の GameObject を生成
		auto bulletObj = std::make_unique<GameObject>(context, "PlayerBullet");

		// MeshRendererComponent を追加してモデルを設定
		auto* meshRenderer = bulletObj->AddComponent<MeshRendererComponent>();
		meshRenderer->SetModel("Resources/bullet/bullet.obj");
		meshRenderer->SetTexture("Resources/bullet/bullet.png");

		// 弾を大きく見やすく拡大表示
		bulletObj->GetTransform().scale = { 2.5f, 2.5f, 2.5f };

		// 水面の下に潜り込んでも深度テストで遮蔽されず、水面越しに弾モデルが常にくっきり描画される設定！
		meshRenderer->SetDepthEnable(false);
		meshRenderer->SetBlendMode(MyEngine::Rendering::BlendModeType::Alpha);

		// 弾の挙動コンポーネントと当たり判定コンポーネントを追加
		auto* bulletComp = bulletObj->AddComponent<BulletComponent>();
		auto* colliderComp = bulletObj->AddComponent<ColliderComponent>();

		// コンポーネントたちを初期化
		bulletObj->Initialize();

		// 生成した弾にプレイヤーで設定しているハープーンのパラメータを適用
		bulletComp->SetSpeed(harpoonSpeed_);
		bulletComp->SetHomingStrength(harpoonHomingStrength_);
		bulletComp->SetMaxDistance(harpoonMaxDistance_);

		// 当たり判定の大きさを設定する（モデル拡大に合わせて 1.2f に拡張）
		colliderComp->SetRadius(1.2f);

		// レティクルがロックしている敵がいれば、弾に追尾対象としてセットする
		if (reticleObject_) {
			if (auto* reticleComp = reticleObject_->GetComponent<ReticleComponent>()) {
				if (auto* target = reticleComp->GetLockOnTarget()) {
					bulletComp->SetTarget(target);
				}
			}
		}

		// 開始位置を設定（プレイヤーの位置）
		bulletObj->GetTransform().translate.x = gameObject_->GetTransform().translate.x;
		bulletObj->GetTransform().translate.y = gameObject_->GetTransform().translate.y + 0.25f;
		bulletObj->GetTransform().translate.z = gameObject_->GetTransform().translate.z;

		// 方向の計算
		Vector3 targetPos = { 0.0f, 0.0f, 100.0f };
		if (reticleObject_) {
			targetPos = reticleObject_->GetTransform().translate;

			// もし敵をロックオンしているなら、その敵の座標を直接狙う
			if (auto* reticleComp = reticleObject_->GetComponent<ReticleComponent>()) {
				if (auto* lockOnEnemy = reticleComp->GetLockOnTarget()) {
					targetPos = lockOnEnemy->GetTransform().translate; // 敵の座標に上書き
				}
			}
		}
		Vector3 direction = Math::Subtract(targetPos, bulletObj->GetTransform().translate);
		if (Math::Length(direction) > 0.001f) {
			direction = Math::Normalize(direction);
		} else {
			direction = { 0.0f, 0.0f, 1.0f };
		}
		bulletComp->SetDirection(direction);

		// 弾が進行方向を向くように回転を設定する
		Vector3 bulletRot = { 0.0f, 0.0f, 0.0f };
		if (Math::Length(direction) > 0.001f) {

			// ヨー回転（左右）の計算
			bulletRot.y = std::atan2(direction.x, direction.z);

			// ピッチ回転（上下）の計算
			float xzLength = std::sqrt(direction.x * direction.x + direction.z * direction.z);
			bulletRot.x = std::atan2(-direction.y, xzLength);
		}
		bulletObj->GetTransform().rotate = bulletRot; // 回転を適用

		// 弾発射時の位置から水飛沫マズルブラストを10個飛び散らせる！
		ParticleSpawner::SpawnWaterSplash(context, 
										{ bulletObj->GetTransform().translate.x, 
										bulletObj->GetTransform().translate.y,
										bulletObj->GetTransform().translate.z
										},
										10);

		// セーブ対象外
		bulletObj->SetSerializable(false);

		// シーンのオブジェクトリストに追加
		context->gameObjects->push_back(std::move(bulletObj));
		cooltime_ = 0.25f;
	}
}
void PlayerComponent::ImGui() {
#ifdef USEIMGUI
	ImGui::Text("--- Player HP Status ---");
	ImGui::DragInt("HP", &hp_, 1, 0, maxHp_);
	ImGui::DragInt("Max HP", &maxHp_, 1, 1, 100);
	float hpRatio = maxHp_ > 0 ? static_cast<float>(hp_) / static_cast<float>(maxHp_) : 0.0f;
	char hpBuf[32];
	sprintf_s(hpBuf, "HP: %d / %d", hp_, maxHp_);
	ImGui::ProgressBar(hpRatio, ImVec2(-1, 0), hpBuf);

	if (ImGui::Button("Test Damage (1 HP)")) {
		TakeDamage(1);
	}
	ImGui::DragFloat("Invincibility Duration", &invincibilityDuration_, 0.1f, 0.0f, 10.0f);

	ImGui::Separator();
	ImGui::DragFloat("Speed (Power)", &speed_, 0.01f, 0.0f, 5.0f);
	ImGui::DragFloat("Max Speed", &maxSpeed_, 0.05f, 0.5f, 10.0f); 
	ImGui::DragFloat("Attenuation (Inertia)", &attenuationRate_, 0.001f, 0.90f, 0.999f);
	ImGui::DragFloat("Brake (Attenuation)", &brakeAttenuationRate_, 0.001f, 0.50f, 0.99f);
	ImGui::DragFloat3("Velocity", &velocity_.x, 0.05f);
	ImGui::DragFloat("turnSpeed", &turnSpeed_, 0.001f);

	if (ImGui::DragFloat("dirRatioZ", &dirRatioZ_, 0.001f, 0.0f, 1.0f)) {
		dirRatioZ_ = std::clamp(dirRatioZ_, 0.0f, 1.0f);
		dirRatioX_ = 1.0f - dirRatioZ_;
	}
	if (ImGui::DragFloat("dirRatioX", &dirRatioX_, 0.001f, 0.0f, 1.0f)) {
		dirRatioX_ = std::clamp(dirRatioX_, 0.0f, 1.0f);
		dirRatioZ_ = 1.0f - dirRatioX_;
	}

	ImGui::Text("--- Brake Behavior Trade-off ---");
	ImGui::SliderFloat("Brake Turn Mult", &brakeTurnSpeedMultiplier_, 1.0f, 10.0f);
	ImGui::SliderFloat("Brake Move Mult", &brakeMoveSpeedMultiplier_, 0.0f, 1.0f);

	ImGui::Text("--- Harpoon Gun ---");
	ImGui::DragFloat("Harpoon Speed", &harpoonSpeed_, 1.0f, 10.0f, 300.0f);
	ImGui::DragFloat("Harpoon Max Distance (Range)", &harpoonMaxDistance_, 0.5f, 5.0f, 100.0f);
	ImGui::SliderFloat("Homing Strength", &harpoonHomingStrength_, 0.0f, 0.5f);
#endif
}

void PlayerComponent::Serialize(json& j) const {
	j["type"] = "PlayerComponent";
	j["hp"] = hp_;
	j["maxHp"] = maxHp_;
	j["speed"] = speed_;
	j["maxSpeed"] = maxSpeed_;
	j["attenuation"] = attenuationRate_;
	j["brakeAttenuation"] = brakeAttenuationRate_;
	j["turnSpeed"] = turnSpeed_;
	j["dirRatioZ"] = dirRatioZ_;
	j["dirRatioX"] = dirRatioX_;
	j["harpoonSpeed"] = harpoonSpeed_;
	j["harpoonHomingStrength"] = harpoonHomingStrength_;
	j["harpoonMaxDistance"] = harpoonMaxDistance_;
	j["brakeTurnSpeedMultiplier"] = brakeTurnSpeedMultiplier_;
	j["brakeMoveSpeedMultiplier"] = brakeMoveSpeedMultiplier_;
}

void PlayerComponent::Deserialize(const json& j) {
	isInitialized_ = true; 

	if(j.contains("hp")) hp_ = j["hp"];
	if(j.contains("maxHp")) maxHp_ = j["maxHp"];
	if(j.contains("harpoonMaxDistance")) harpoonMaxDistance_ = j["harpoonMaxDistance"];

	if(j.contains("speed")) {
		speed_ = j["speed"];
	}
	if(j.contains("maxSpeed")) {
		maxSpeed_ = j["maxSpeed"];
	}
	if(j.contains("attenuation")) {
		attenuationRate_ = j["attenuation"];
	}
	if(j.contains("brakeAttenuation")) {
		brakeAttenuationRate_ = j["brakeAttenuation"];
	}
	if(j.contains("turnSpeed")) {
		turnSpeed_ = j["turnSpeed"];
	}
	if(j.contains("dirRatioZ")) {
		dirRatioZ_ = j["dirRatioZ"];
	}
	if(j.contains("dirRatioX")) {
		dirRatioX_ = j["dirRatioX"];
	}
	if(j.contains("harpoonSpeed")) {
		harpoonSpeed_ = j["harpoonSpeed"];
	}
	if(j.contains("homingStrength")) {
		harpoonHomingStrength_ = j["homingStrength"];
	}
	if(j.contains("brakeTurnSpeedMultiplier")) {
		brakeTurnSpeedMultiplier_ = j["brakeTurnSpeedMultiplier"];
	}
	if(j.contains("brakeMoveSpeedMultiplier")) {
		brakeMoveSpeedMultiplier_ = j["brakeMoveSpeedMultiplier"];
	}
}
