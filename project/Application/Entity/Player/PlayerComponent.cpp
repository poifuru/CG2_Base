#include "PCH.h"
#include "PlayerComponent.h"
#include "GameObject.h"
#include "InputManager.h"
#include "RawInput.h"
#include "DeltaTime.h"
#include "MathFunction.h"
#include "CameraOrganizer.h"
#include "BaseCamera.h" // CameraDataの定義があるヘッダー
#include "MeshRendererComponent.h"
#include "BulletComponent.h"
#include "BaseScene.h"
#include "ReticleComponent.h"
#include "ColliderComponent.h"

// キー入力が無いときに速度を減衰させる定数
static const float kAttenuationRate = 0.95f;

void PlayerComponent::Initialize() {
	speed_ = 1.5f;
	velocity_ = { 0.0f, 0.0f, 0.0f };
	cooltime_ = 0.0f;

	// 初期位置を水中に設定（Y = -10.0f）
	gameObject_->GetTransform().translate = { 0.0f, 0.3f, 0.0f };
}

void PlayerComponent::Update() {
	if(isDebugMode_) {
		return;
	}

	// クールタイム更新
	if(cooltime_ > 0.0f) {
		cooltime_ -= kDeltaTime;
	}

	// 移動処理
	Move();

	// 射撃処理
	Shoot();
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
	InputManager* input = InputManager::GetInstance();
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

	// Spaceで上昇、Left Shiftで下降
	/*if(input->GetRawInput()->Push(VK_SPACE)) { moveDir.y += 1.0f; }
	if(input->GetRawInput()->Push(VK_SHIFT)) { moveDir.y -= 1.0f; }*/

	// 入力があった場合に移動と回転を設定する
	if(Math::Length(moveDir) > 0.0f) {
		moveDir = Math::Normalize(moveDir);

		// キャラクターの向き（Yaw/Pitch）を移動方向に合わせる
		gameObject_->GetTransform().rotate.y = std::atan2(moveDir.x, moveDir.z);
		float xzLength = std::sqrt(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
		gameObject_->GetTransform().rotate.x = std::atan2(-moveDir.y, xzLength);
		// 加速度を設定
		acceleration_.x = moveDir.x * speed_ * kDeltaTime;
		acceleration_.y = moveDir.y * speed_ * kDeltaTime;
		acceleration_.z = moveDir.z * speed_ * kDeltaTime;
		velocity_.x += acceleration_.x;
		velocity_.y += acceleration_.y;
		velocity_.z += acceleration_.z;
	}

	// 速度の減衰と制限
	velocity_.x *= kAttenuationRate;
	velocity_.y *= kAttenuationRate;
	velocity_.z *= kAttenuationRate;

	const float maxSpeed = 5.0f;
	velocity_.x = std::clamp(velocity_.x, -maxSpeed, maxSpeed);
	velocity_.y = std::clamp(velocity_.y, -maxSpeed, maxSpeed);
	velocity_.z = std::clamp(velocity_.z, -maxSpeed, maxSpeed);

	const float minSpeed = 0.005f;
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
}

void PlayerComponent::Shoot() {
	InputManager* input = InputManager::GetInstance();

	if (input->GetRawInput()->TriggerMouse(0) && cooltime_ <= 0.0f) {
		auto* context = gameObject_->GetContext();
		if (!context || !context->gameObjects) return;

		// 弾用の GameObject を生成
		auto bulletObj = std::make_unique<GameObject>(context, "PlayerBullet");

		// MeshRendererComponent を追加してモデルを設定（これで描画される！）
		auto* meshRenderer = bulletObj->AddComponent<MeshRendererComponent>();
		meshRenderer->SetModel("Resources/bullet/bullet.obj");
		meshRenderer->SetTexture("Resources/bullet/bullet.png");

		// 弾の挙動コンポーネントを追加
		auto* bulletComp = bulletObj->AddComponent<BulletComponent>();

		// 弾にコライダーを追加して、当たり判定の大きさを設定する！
		auto* colliderComp = bulletObj->AddComponent<ColliderComponent>();
		colliderComp->SetRadius(0.5f); // 弾の半径を 0.5m など適当なサイズにする

		// 開始位置を設定（プレイヤーの位置）
		bulletObj->GetTransform().translate = gameObject_->GetTransform().translate;

		// 方向の計算
		Vector3 targetPos = { 0.0f, 0.0f, 100.0f };
		if (reticleObject_) {
			targetPos = reticleObject_->GetTransform().translate;
		}
		Vector3 direction = Math::Subtract(targetPos, bulletObj->GetTransform().translate);
		if (Math::Length(direction) > 0.001f) {
			direction = Math::Normalize(direction);
		} else {
			direction = { 0.0f, 0.0f, 1.0f };
		}
		bulletComp->SetDirection(direction);

		// 弾が進行方向を向くように回転を設定する！
		Vector3 bulletRot = { 0.0f, 0.0f, 0.0f };
		if (Math::Length(direction) > 0.001f) {

			// ヨー回転（左右）の計算
			bulletRot.y = std::atan2(direction.x, direction.z);

			// ピッチ回転（上下）の計算
			float xzLength = std::sqrt(direction.x * direction.x + direction.z * direction.z);
			bulletRot.x = std::atan2(-direction.y, xzLength);
		}
		bulletObj->GetTransform().rotate = bulletRot; // 回転を適用！

		// 初期化して、シーンのオブジェクトリストに追加！
		bulletObj->Initialize();
		context->gameObjects->push_back(std::move(bulletObj));
		cooltime_ = 0.25f;
	}
}
void PlayerComponent::ImGui() {
#ifdef USEIMGUI
	ImGui::DragFloat("Speed", &speed_, 0.05f);
	ImGui::DragFloat3("Velocity", &velocity_.x, 0.05f);
#endif
}

void PlayerComponent::Serialize(json& j) const {
	j["type"] = "PlayerComponent";
	j["speed"] = speed_;
}

void PlayerComponent::Deserialize(const json& j) {
	if(j.contains("speed")) {
		speed_ = j["speed"];
	}
}
