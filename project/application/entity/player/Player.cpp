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
	TextureManager::GetInstance()->LoadTexture("Resources/monsterBall/monsterBall.png", "bullet");
	ModelManager::GetInstance()->LoadModelData("Resources/monsterBall", "monsterBall.obj");

	// Reticle用
	TextureManager::GetInstance()->LoadTexture("Resources/reticle/reticle.png", "reticle");
	ModelManager::GetInstance()->LoadModelData("Resources/reticle", "reticle.obj");

	model_ = std::make_unique<Model>(dxCommon, light);
	reticle_ = std::make_unique<Reticle>(dxCommon, camera, input, light);
}

Player::~Player() {

}

void Player::Initialize() {
	model_->SetModelData("AnimatedCube.gltf");
	model_->SetTexture("Cube");
	//model_->SetAnimation("AnimatedCube.gltf");
	model_->Initialize();

	// 固有の数値
	speed_ = 1.5f;
	velocity_ = { 0.0f, 0.0f, 0.0f };
	cooltime_ = 0.0f;
	localTranslate_ = { 0.0f, 0.0f, 0.0f };
	lockedEnemy_ = nullptr;

	reticle_->Initialize();
}

void Player::Update() {
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
			
			// --- カメラの後ろにいる敵は除外する (カメラの前方向ベクトルとの内積で判定) ---
			Vector3 cameraPos = camera_->GetCameraData().transform.translate;
			Matrix4x4 camWorld = camera_->GetCameraData().world;
			Vector3 cameraForward = { camWorld.m[2][0], camWorld.m[2][1], camWorld.m[2][2] };
			Vector3 toEnemy = Math::Subtract(enemyPos, cameraPos);
			
			if (Math::Dot(toEnemy, cameraForward) < 0.0f) continue; // カメラの後ろは無視
			
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

	// レールがある場合、レールの進行方向を向くように回転を設定する
	if (railPath_) {
		Matrix4x4 rot = railPath_->GetRotationMatrix();
		Vector3 direction = { rot.m[2][0], rot.m[2][1], rot.m[2][2] }; // Z軸の方向
		
		Vector3 rotate = { 0.0f, 0.0f, 0.0f };
		if (Math::Length(direction) > 0.001f) {
			rotate.y = std::atan2(direction.x, direction.z);
			float xzLength = std::sqrt(direction.x * direction.x + direction.z * direction.z);
			rotate.x = std::atan2(-direction.y, xzLength);
		}
		model_->SetRotate(rotate);
	}

	model_->Update(&camera_->GetCameraData());
}

void Player::Draw() {
	model_->Draw();
	BulletsDraw();
	reticle_->Draw();
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

	// ローカル座標の更新
	localTranslate_.x += velocity_.x;
	localTranslate_.y += velocity_.y;

	// カメラの画角内に移動を制限
	if (camera_) {
		// カメラとプレイヤー（レール）の現在のワールド座標を取得
		Vector3 cameraPos = camera_->GetCameraData().transform.translate;
		Vector3 basePos = transform_.translate;
		if (railPath_) {
			basePos = railPath_->GetPosition();
		}

		// カメラとレールの「実際の距離（Z軸）」を計算
		float distanceToCamera = basePos.z - cameraPos.z;
		if (distanceToCamera <= 0.0f) {
			distanceToCamera = 50.0f;
		}

		// 画角とアスペクト比から画面の半分サイズを計算 (FOV: 0.45f)
		float fovY = 0.45f; 
		float aspect = 1280.0f / 720.0f; 

		float halfHeight = std::tan(fovY * 0.5f) * distanceToCamera;
		float halfWidth = halfHeight * aspect;

		// プレイヤーのモデルサイズに合わせたマージン
		float marginX = 2.0f;
		float marginY = 2.0f;

		// 左右の制限幅（X軸はカメラが真ん中なので左右対称でOK）
		float limitX = (std::max)(0.0f, halfWidth - marginX);

		// カメラの高さのズレ（オフセット）を計算する
		// レールの高さ（basePos.y）に対して、カメラがどれだけ高い位置にいるか
		float cameraHeightOffset = cameraPos.y - basePos.y; // PlaySceneの設定通りなら約3.0fになる

		// 本来の画面端（halfHeight）に対して、カメラが上に上がった分だけ制限を「下にシフト」させる
		float limitTop    =  halfHeight - marginY + cameraHeightOffset; // 上方向の限界（引き上げられるので広くなる）
		float limitBottom = -halfHeight + marginY + cameraHeightOffset; // 下方向の限界（引き上げられるので狭くなる）

		// 上下のバウンドが逆転しないように最低限の幅を保証
		if (limitTop < limitBottom) {
			limitTop = marginY;
			limitBottom = -marginY;
		}

		// クランプ処理
		// X軸は今まで通り（左右対称）
		localTranslate_.x = std::clamp(localTranslate_.x, -limitX, limitX);
		// Y軸はカメラの高さで補正した「個別の上下限値」でクランプ！
		localTranslate_.y = std::clamp(localTranslate_.y, limitBottom, limitTop);
	}

	if (railPath_) {
		Vector3 railPos = railPath_->GetPosition();
		Matrix4x4 railRot = railPath_->GetRotationMatrix();

		// ローカルのズレをレールの向きで回転させ、レールの位置と足し合わせる
		Vector3 rotatedLocal = Math::Transform(localTranslate_, railRot);
		transform_.translate = Math::Add(railPos, rotatedLocal);
	} else {
		// レールが無いときのフォールバック（デバッグ用）
		transform_.translate.x += velocity_.x;
		transform_.translate.y += velocity_.y;
		transform_.translate.z += velocity_.z * kDeltaTime;
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