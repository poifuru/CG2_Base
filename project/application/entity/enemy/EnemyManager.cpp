#include "EnemyManager.h"
#include "SmallFish.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "MathFunction.h"
#include <algorithm>

void EnemyManager::Initialize(DxCommon* dxCommon, LightManager* light, CameraOrganizer* camera) {
	dxCommon_ = dxCommon;
	light_ = light;
	camera_ = camera;

	// 敵のモデルやテクスチャのロード
	TextureManager::GetInstance()->LoadTexture("Resources/teapot/teapot.png", "teapot");
	ModelManager::GetInstance()->LoadModelData("Resources/teapot", "teapot.obj");

	// 魚モデルのロード
	ModelManager::GetInstance()->LoadModelData("Resources/enemy/smallFish", "smallFish.obj");


	// プールを全てSmallFishで初期化（非アクティブ状態）
	for (auto& enemy : enemies_) {
		enemy = std::make_unique<SmallFish>(dxCommon_, light_, camera_);
		enemy->Initialize();
		enemy->SetIsActive(false);
	}

	// --- スポーンタイムライン（仮置き） ---
	// TPS水中化: Y座標を水中（-8.0f 〜 -12.0f）に変更
	spawnTimeline_ = {
		{ 10.0f, kSmallFish, { -5.0f, -10.0f, 30.0f } },
		{ 10.0f, kSmallFish, {  0.0f,  -8.0f, 30.0f } },
		{ 10.0f, kSmallFish, {  5.0f, -12.0f, 30.0f } },
		{ 30.0f, kSmallFish, { -3.0f, -10.0f, 35.0f } },
		{ 30.0f, kSmallFish, {  3.0f,  -9.0f, 35.0f } },
		{ 50.0f, kSmallFish, {  0.0f, -11.0f, 40.0f } },
	};
	currentSpawnIndex_ = 0;

	// TPS化: 最初からすべての敵をスポーンさせておく
	for (const auto& cmd : spawnTimeline_) {
		SpawnEnemy(cmd.enemyType, cmd.startPos, nullptr);
	}
}

void EnemyManager::Update(const Vector3& playerPos, const RailPath* railPath) {
	// 出現予定リストからの自動スポーンは行わない（Initializeで一括生成するため）

	// 2. 生きている敵の更新
	if (camera_) {
		Matrix4x4 camWorld = camera_->GetCameraData().world;
		Vector3 cameraForward = { camWorld.m[2][0], camWorld.m[2][1], camWorld.m[2][2] };
		Vector3 cameraPos = camera_->GetCameraData().transform.translate;

		for (auto& enemy : enemies_) {
			if (enemy && enemy->IsActive()) {
				// TPS化: カメラの手前や背後に回り込んでも消さず、通常通り更新する
				enemy->SetScale({ 1.0f, 1.0f, 1.0f });
				enemy->SetAlpha(1.0f);
				enemy->Update();
			}
		}
	} else {
		for (auto& enemy : enemies_) {
			if (enemy && enemy->IsActive()) {
				enemy->Update();
			}
		}
	}
}

void EnemyManager::Draw() {
	for (auto& enemy : enemies_) {
		if (enemy && enemy->IsActive()) {
			enemy->Draw();
		}
	}
}

void EnemyManager::SpawnEnemy(int type, const Vector3& pos, const RailPath* railPath) {
	// プールから非アクティブな敵を探す
	for (auto& enemy : enemies_) {
		if (enemy && !enemy->IsActive()) {
			if (railPath) {
				Vector3 railPos = railPath->GetPosition();
				Matrix4x4 railRot = railPath->GetRotationMatrix();
				// レールからの相対座標（pos）をレールの向きで回転させ、ワールド座標にする
				Vector3 worldPos = Math::Add(railPos, Math::Transform(pos, railRot));
				enemy->SetPosition(worldPos);
			} else {
				enemy->SetPosition(pos);
			}
			enemy->SetIsActive(true);
			return;
		}
	}
	// 空きがなければスポーンしない（プール満杯）
}
