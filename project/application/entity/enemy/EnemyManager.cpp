#include "EnemyManager.h"
#include "SmallFish.h"
#include "TextureManager.h"
#include "ModelManager.h"

void EnemyManager::Initialize(DxCommon* dxCommon, LightManager* light, CameraOrganizer* camera) {
	dxCommon_ = dxCommon;
	light_ = light;
	camera_ = camera;

	// 敵のモデルやテクスチャのロード
	TextureManager::GetInstance()->LoadTexture("Resources/teapot/teapot.png", "teapot");
	ModelManager::GetInstance()->LoadModelData("Resources/teapot", "teapot.obj");


	// プールを全てSmallFishで初期化（非アクティブ状態）
	for (auto& enemy : enemies_) {
		enemy = std::make_unique<SmallFish>(dxCommon_, light_, camera_);
		enemy->Initialize();
		enemy->SetIsActive(false);
	}

	// --- スポーンタイムライン（仮置き） ---
	spawnTimeline_ = {
		{ 10.0f, kSmallFish, { -5.0f, 0.0f, 30.0f } },
		{ 10.0f, kSmallFish, {  0.0f, 0.0f, 30.0f } },
		{ 10.0f, kSmallFish, {  5.0f, 0.0f, 30.0f } },
		{ 30.0f, kSmallFish, { -3.0f, 0.0f, 35.0f } },
		{ 30.0f, kSmallFish, {  3.0f, 0.0f, 35.0f } },
		{ 50.0f, kSmallFish, {  0.0f, 0.0f, 40.0f } },
	};
	currentSpawnIndex_ = 0;
}

void EnemyManager::Update(float playerProgressZ) {
	// 出現予定リストを見てプレイヤーが一定距離進んでいたらスポーン
	while (currentSpawnIndex_ < spawnTimeline_.size()) {
		const auto& cmd = spawnTimeline_[currentSpawnIndex_];
		if (playerProgressZ >= cmd.triggerZ) {
			SpawnEnemy(cmd.enemyType, cmd.startPos);
			currentSpawnIndex_++;
		}
		else {
			break; // まだ出撃タイミングじゃないならループを抜ける
		}
	}
	// 2. 生きている敵の更新
	for (auto& enemy : enemies_) {
		if (enemy && enemy->IsActive()) { // アクティブな（画面にいる）敵だけ更新
			enemy->Update();
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

void EnemyManager::SpawnEnemy(int type, const Vector3& pos) {
	// プールから非アクティブな敵を探す
	for (auto& enemy : enemies_) {
		if (enemy && !enemy->IsActive()) {
			enemy->SetPosition(pos);
			enemy->SetIsActive(true);
			return;
		}
	}
	// 空きがなければスポーンしない（プール満杯）
}
