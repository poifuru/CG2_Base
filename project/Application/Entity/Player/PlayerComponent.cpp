#include "PCH.h"
#include "PlayerComponent.h"
#include "GameObject.h"
#include "InputManager.h"
#include "DeltaTime.h"
#include "MathFunction.h"

// キー入力が無いときに速度を減衰させる定数
static const float kAttenuationRate = 0.95f;

void PlayerComponent::Initialize() {
	speed_ = 1.5f;
	velocity_ = { 0.0f, 0.0f, 0.0f };
	cooltime_ = 0.0f;
	localTranslate_ = { 0.0f, 0.0f, 0.0f };

	// 親の GameObject から現在の位置をローカル座標の初期値にする
	localTranslate_ = gameObject_->GetTransform().translate;
}

void PlayerComponent::Update() {
	// クールタイム更新
	if (cooltime_ > 0.0f) {
		cooltime_ -= kDeltaTime;
	}
	// 移動処理
	Move();
	// 射撃処理
	Shoot();
}
//
//void Player::Draw() {
//	if(camera_->GetActiveCameraName() != "main2") {
//		return;
//	}
//
//	model_->Draw();
//	BulletsDraw();
//}
//
//void Player::DrawUI() {
//	if(camera_->GetActiveCameraName() != "main2") {
//		return;
//	}
//
//	reticle_->Draw();
//}
//
//void Player::Input() {
//	// *** 移動入力 *** //
//	// 加速度をリセット
//	acceleration_.x = 0.0f;
//	acceleration_.y = 0.0f;
//
//	// フレーム内の入力を方向として蓄積
//	Vector2 moveDir = { 0.0f, 0.0f };
//
//	if(input_->GetRawInput()->Push('W')) { moveDir.y += 1.0f; }
//	if(input_->GetRawInput()->Push('S')) { moveDir.y -= 1.0f; }
//	if(input_->GetRawInput()->Push('A')) { moveDir.x -= 1.0f; }
//	if(input_->GetRawInput()->Push('D')) { moveDir.x += 1.0f; }
//
//	// 入力があった場合に処理する
//	if(moveDir.x != 0.0f || moveDir.y != 0.0f) {
//		// ベクトルの長さを計算
//		float length = std::sqrt(moveDir.x * moveDir.x + moveDir.y * moveDir.y);
//
//		// 正規化
//		moveDir.x /= length;
//		moveDir.y /= length;
//
//		// 実際に速度、デルタタイムを掛ける
//		acceleration_.x = moveDir.x * speed_ * kDeltaTime;
//		acceleration_.y = moveDir.y * speed_ * kDeltaTime;
//
//		velocity_.x += acceleration_.x;
//		velocity_.y += acceleration_.y;
//	}
//	// ****** //
//
//	// *** 弾の発射 *** //
//	const float kCooltime = 0.25f;	// 連射間隔の制限
//	const uint32_t kMaxBulletCount = 5;	// 同時に存在できる弾の最大数
//
//	if(input_->GetRawInput()->Trigger(VK_SPACE) &&
//	   bullets_.size() < kMaxBulletCount &&
//	   cooltime_ <= 0.0f) {	// スペースキーを押した瞬間
//		// 新しい弾を生成&初期化
//		std::unique_ptr<Bullet> newBullet = std::make_unique<Bullet>(dxCommon_, camera_, input_, light_);
//		newBullet->Initialize();
//
//		// *** レティクルに向けて飛ばす処理 *** //
//
//		// プレイヤーの現在地を取得してtranslateをセット
//		Vector3 startPos = transform_.translate;
//		newBullet->SetTranslate(startPos);
//
//		// レティクルの現在地を取得
//		Vector3 targetPos = reticle_->GetPosition();
//
//		// 方向ベクトルの計算
//		Vector3 direction = {
//			targetPos.x - startPos.x,
//			targetPos.y - startPos.y,
//			targetPos.z - startPos.z,
//		};
//
//		// 方向ベクトルを正規化する
//		float length = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
//
//		if(length > 0.0f) {
//			direction.x /= length;
//			direction.y /= length;
//			direction.z /= length;
//		}
//		else {	// プレイヤーとレティクルの座標が同じの場合
//			direction = { 0.0f, 0.0f, 1.0f };
//		}
//
//		// 求めた数値をBulletに渡す
//		newBullet->SetDirection(direction);
//		newBullet->SetPlayer(this);
//
//		if (lockedEnemy_) {
//			newBullet->SetTarget(lockedEnemy_);
//		}
//
//		// ****** //
//
//		// リストに追加
//		bullets_.push_back(std::move(newBullet));
//
//		// クールタイムを設定
//		cooltime_ = kCooltime;
//	}
//	// ****** //
//}
//
//void Player::CooltimeUpdate() {
//	if(cooltime_ > 0.0f) {
//		cooltime_ -= kDeltaTime;
//	}
//}
//
//void
//Player::Move() {
//	// 速度に減衰率をかけ続ける
//	velocity_.x *= kAttenuationRate;
//	velocity_.y *= kAttenuationRate;
//
//	// 速度が上限、下限に触れていないかチェック
//	const float maxSpeed = 5.0f;
//	velocity_.x = std::clamp(velocity_.x, -maxSpeed, maxSpeed);
//	velocity_.y = std::clamp(velocity_.y, -maxSpeed, maxSpeed);
//
//	// 減衰して速度が一定以下になったら0とみなす
//	// 速度の最低値
//	const float minSpeed = 0.005f;
//
//	if(std::abs(velocity_.x) < minSpeed) {
//		velocity_.x = 0.0f;
//	}
//	if(std::abs(velocity_.y) < minSpeed) {
//		velocity_.y = 0.0f;
//	}
//
//	// ローカル座標の更新
//	localTranslate_.x += velocity_.x;
//	localTranslate_.y += velocity_.y;
//
//	// カメラの画角内に移動を制限
//	if (camera_) {
//		// 1. クランプ前の予測ワールド位置を計算する
//		Vector3 worldPos = transform_.translate;
//		if (railPath_) {
//			Vector3 railPos = railPath_->GetPosition();
//			Matrix4x4 railRot = railPath_->GetRotationMatrix();
//			Vector3 rotatedLocal = Math::Transform(localTranslate_, railRot);
//			worldPos = Math::Add(railPos, rotatedLocal);
//		}
//
//		// 2. ビュー空間に変換する
//		Matrix4x4 viewMat = camera_->GetCameraData().view;
//		Vector3 viewPos = Math::Transform(worldPos, viewMat);
//
//		// 3. ビュー空間のZ軸（カメラからの距離）を基準に、制限限界値を計算
//		float distanceToCamera = viewPos.z;
//		if (distanceToCamera <= 0.0f) {
//			distanceToCamera = 50.0f;
//		}
//
//		float fovY = 0.45f;
//		float aspect = 1280.0f / 720.0f;
//
//		float halfHeight = std::tan(fovY * 0.5f) * distanceToCamera;
//		float halfWidth = halfHeight * aspect;
//
//		float marginX = 2.0f;
//		float marginY = 2.0f;
//
//		float limitX = (std::max)(0.0f, halfWidth - marginX);
//		float limitY = (std::max)(0.0f, halfHeight - marginY);
//
//		// 4. ビュー空間上でクランプ
//		viewPos.x = std::clamp(viewPos.x, -limitX, limitX);
//		viewPos.y = std::clamp(viewPos.y, -limitY, limitY);
//
//		// 5. ワールド座標に戻す
//		Matrix4x4 cameraWorld = camera_->GetCameraData().world;
//		worldPos = Math::Transform(viewPos, cameraWorld);
//
//		// 6. レールのローカル座標 (localTranslate_) に逆変換する
//		if (railPath_) {
//			Vector3 railPos = railPath_->GetPosition();
//			Matrix4x4 railRot = railPath_->GetRotationMatrix();
//			Matrix4x4 invRailRot = Math::Inverse(railRot);
//
//			Vector3 diff = Math::Subtract(worldPos, railPos);
//			localTranslate_ = Math::Transform(diff, invRailRot);
//			localTranslate_.z = 0.0f; // レール前進方向へのズレは無効化
//		} else {
//			transform_.translate = worldPos;
//		}
//	}
//
//	if (railPath_) {
//		Vector3 railPos = railPath_->GetPosition();
//		Matrix4x4 railRot = railPath_->GetRotationMatrix();
//
//		// ローカルのズレをレールの向きで回転させ、レールの位置と足し合わせる
//		Vector3 rotatedLocal = Math::Transform(localTranslate_, railRot);
//		transform_.translate = Math::Add(railPos, rotatedLocal);
//	} else {
//		// レールが無いときのフォールバック（デバッグ用）
//		transform_.translate.x += velocity_.x;
//		transform_.translate.y += velocity_.y;
//		transform_.translate.z += velocity_.z * kDeltaTime;
//	}
//}
//
//void Player::BulletsUpdate() {
//	// イテレータでループを回す
//	for(auto it = bullets_.begin(); it != bullets_.end();) {
//		// 更新
//		(*it)->Update();
//
//		// 弾の寿命が過ぎていたらリストから削除
//		if(!(*it)->IsActive()) {
//			it = bullets_.erase(it);
//		}
//		else {
//			// 削除されなかったら次に進む
//			++it;
//		}
//	}
//}
//
//void Player::BulletsDraw() {
//	for(auto it = bullets_.begin(); it != bullets_.end(); ++it) {
//		(*it)->Draw();
//	}
//}
//
//void Player::ImGui() {
//#ifdef USEIMGUI
//	ImGui::Begin("Player");
//	ImGui::DragFloat3("Position", &transform_.translate.x, 0.01f);
//	ImGui::DragFloat3("Velocity", &velocity_.x, 0.01f);
//	ImGui::DragFloat3("Acceleration", &acceleration_.x, 0.01f);
//	
//	ImGui::Separator();
//	ImGui::Text("--- LockOn Debug ---");
//	if (lockedEnemy_) {
//		ImGui::Text("Locked Enemy: Yes (Address: %p)", lockedEnemy_);
//	} else {
//		ImGui::Text("Locked Enemy: No");
//	}
//	
//	if (enemyManager_) {
//		auto& enemies = enemyManager_->GetEnemies();
//		int activeCount = 0;
//		for (const auto& e : enemies) {
//			if (e && e->IsActive()) activeCount++;
//		}
//		ImGui::Text("Active Enemies: %d / %d", activeCount, (int)enemies.size());
//			
//		if (camera_ && reticle_) {
//			Matrix4x4 vpMat = camera_->GetVPMatrix();
//			Vector3 retPos = reticle_->GetPosition();
//			Vector3 retNdc = Math::ChangeTransform(retPos, vpMat);
//			ImGui::Text("Reticle World: (%.1f, %.1f, %.1f)", retPos.x, retPos.y, retPos.z);
//			ImGui::Text("Reticle NDC: (%.3f, %.3f, %.3f)", retNdc.x, retNdc.y, retNdc.z);
//			
//			int idx = 0;
//			for (auto& enemy : enemies) {
//				if (enemy && enemy->IsActive()) {
//					Vector3 ePos = enemy->GetTransform().translate;
//					Vector3 eNdc = Math::ChangeTransform(ePos, vpMat);
//					ImGui::Text("Enemy[%d] World: (%.1f, %.1f, %.1f)", idx, ePos.x, ePos.y, ePos.z);
//					ImGui::Text("Enemy[%d] NDC: (%.3f, %.3f, %.3f)", idx, eNdc.x, eNdc.y, eNdc.z);
//					idx++;
//				}
//			}
//		}
//	}
//	ImGui::End();
//#endif
//}