//#include "EnemyManager.h"
//#include "SmallFish.h"
//#include "TextureManager.h"
//#include "ModelManager.h"
//#include "MathFunction.h"
//#include <algorithm>
//
//void EnemyManager::Initialize(DxCommon* dxCommon, LightManager* light, CameraOrganizer* camera) {
//	dxCommon_ = dxCommon;
//	light_ = light;
//	camera_ = camera;
//
//	// 敵のモデルやテクスチャのロード
//	TextureManager::GetInstance()->LoadTexture("Resources/teapot/teapot.png", "teapot");
//	ModelManager::GetInstance()->LoadModelData("Resources/teapot", "teapot.obj");
//
//
//	// プールを全てSmallFishで初期化（非アクティブ状態）
//	for (auto& enemy : enemies_) {
//		enemy = std::make_unique<SmallFish>(dxCommon_, light_, camera_);
//		enemy->Initialize();
//		enemy->SetIsActive(false);
//	}
//
//	// --- スポーンタイムライン（仮置き） ---
//	spawnTimeline_ = {
//		{ 10.0f, kSmallFish, { -5.0f, 0.0f, 30.0f } },
//		{ 10.0f, kSmallFish, {  0.0f, 0.0f, 30.0f } },
//		{ 10.0f, kSmallFish, {  5.0f, 0.0f, 30.0f } },
//		{ 30.0f, kSmallFish, { -3.0f, 0.0f, 35.0f } },
//		{ 30.0f, kSmallFish, {  3.0f, 0.0f, 35.0f } },
//		{ 50.0f, kSmallFish, {  0.0f, 0.0f, 40.0f } },
//	};
//	currentSpawnIndex_ = 0;
//}
//
//void EnemyManager::Update(const Vector3& playerPos, const RailPath* railPath) {
//	// 出現予定リストを見てプレイヤーが一定距離進んでいたらスポーン
//	while (currentSpawnIndex_ < spawnTimeline_.size()) {
//		const auto& cmd = spawnTimeline_[currentSpawnIndex_];
//		if (playerPos.z >= cmd.triggerZ) {
//			SpawnEnemy(cmd.enemyType, cmd.startPos, railPath);
//			currentSpawnIndex_++;
//		}
//		else {
//			break; // まだ出撃タイミングじゃないならループを抜ける
//		}
//	}
//	// 2. 生きている敵の更新
//	if (camera_) {
//		Matrix4x4 camWorld = camera_->GetCameraData().world;
//		Vector3 cameraForward = { camWorld.m[2][0], camWorld.m[2][1], camWorld.m[2][2] };
//		Vector3 cameraPos = camera_->GetCameraData().transform.translate;
//
//		// プレイヤーのカメラ前方向への距離（深度）
//		float playerDist = Math::Dot(Math::Subtract(playerPos, cameraPos), cameraForward);
//
//		for (auto& enemy : enemies_) {
//			if (enemy && enemy->IsActive()) {
//				Vector3 enemyPos = enemy->GetTransform().translate;
//				float enemyDist = Math::Dot(Math::Subtract(enemyPos, cameraPos), cameraForward);
//
//				// プレイヤーより手前（カメラ側）に回り込んだ場合のスケールダウン
//				float scaleFactor = 1.0f;
//				if (enemyDist < playerDist) {
//					float diff = playerDist - enemyDist;
//					scaleFactor = 1.0f - std::clamp(diff / 10.0f, 0.0f, 1.0f);
//				}
//				enemy->SetScale({ scaleFactor, scaleFactor, scaleFactor });
//				enemy->SetAlpha(1.0f); // 念のためアルファは不透明にしておく
//
//				// カメラの目の前（5.0f以下）まで通り過ぎたら非アクティブ化して消去
//				if (enemyDist < 5.0f) {
//					enemy->SetIsActive(false);
//				} else {
//					enemy->Update();
//				}
//			}
//		}
//	} else {
//		for (auto& enemy : enemies_) {
//			if (enemy && enemy->IsActive()) {
//				enemy->Update();
//			}
//		}
//	}
//}
//
//void EnemyManager::Draw() {
//	for (auto& enemy : enemies_) {
//		if (enemy && enemy->IsActive()) {
//			enemy->Draw();
//		}
//	}
//}
//
//void EnemyManager::SpawnEnemy(int type, const Vector3& pos, const RailPath* railPath) {
//	// プールから非アクティブな敵を探す
//	for (auto& enemy : enemies_) {
//		if (enemy && !enemy->IsActive()) {
//			if (railPath) {
//				Vector3 railPos = railPath->GetPosition();
//				Matrix4x4 railRot = railPath->GetRotationMatrix();
//				// レールからの相対座標（pos）をレールの向きで回転させ、ワールド座標にする
//				Vector3 worldPos = Math::Add(railPos, Math::Transform(pos, railRot));
//				enemy->SetPosition(worldPos);
//			} else {
//				enemy->SetPosition(pos);
//			}
//			enemy->SetIsActive(true);
//			return;
//		}
//	}
//	// 空きがなければスポーンしない（プール満杯）
//}
