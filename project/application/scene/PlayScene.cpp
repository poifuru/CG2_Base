#include "PlayScene.h"
#include "MathFunction.h"
#include "CameraOrganizer.h"
#include "InputManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "imgui.h"

#include "Korokoro.h"
#include "Fly.h"

PlayScene::PlayScene () {
	TextureManager::GetInstance()->LoadTexture("Resources/map/map1.png", "map1");
	TextureManager::GetInstance()->LoadTexture("Resources/map/map2.png", "map2");
	TextureManager::GetInstance()->LoadTexture("Resources/map/map3.png", "map3");
	TextureManager::GetInstance()->LoadTexture("Resources/map/map4.png", "map4");
	TextureManager::GetInstance()->LoadTexture("Resources/map/map5.png", "map5");
	TextureManager::GetInstance()->LoadTexture("Resources/life.png", "life");
	ModelManager::GetInstance()->LoadModelData("Resources/player", "player.obj");
	TextureManager::GetInstance()->LoadTexture("Resources/player/player.png", "player");
	ModelManager::GetInstance()->LoadModelData("Resources/hammer", "hammer.obj");
	TextureManager::GetInstance()->LoadTexture("Resources/hammer/hammer.png", "hammer");
	ModelManager::GetInstance()->LoadModelData("Resources/korokoro", "korokoro.obj");
	TextureManager::GetInstance()->LoadTexture("Resources/korokoro/korokoro.png", "korokoro");
	TextureManager::GetInstance()->LoadTexture("Resources/sousa.png", "sousa");
	ModelManager::GetInstance()->LoadModelData("Resources/skydome", "skydome.obj");
	TextureManager::GetInstance()->LoadTexture("Resources/skydome/skydome.png", "skydome");
}

PlayScene::~PlayScene () {
	TextureManager::GetInstance()->UnloadTexture("Resources/map/map1.png");
	TextureManager::GetInstance()->UnloadTexture("Resources/map/map2.png");
	TextureManager::GetInstance()->UnloadTexture("Resources/map/map3.png");
	TextureManager::GetInstance()->UnloadTexture("Resources/map/map4.png");
	TextureManager::GetInstance()->UnloadTexture("Resources/map/map5.png");
	TextureManager::GetInstance()->UnloadTexture("Resources/life.png");
	ModelManager::GetInstance()->UnloadModelData("player.obj");
	TextureManager::GetInstance()->UnloadTexture("Resources/player/player.png");
	ModelManager::GetInstance()->UnloadModelData("hammer.obj");
	TextureManager::GetInstance()->UnloadTexture("Resources/hammer/hammer.png");
	TextureManager::GetInstance()->UnloadTexture("Resources/sousa.png");
	//ModelManager::GetInstance()->UnloadModelData("skydome.obj");
	//TextureManager::GetInstance()->UnloadTexture("Resources/skydome/skydome.png");
}

void PlayScene::Initialize (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	camera_ = camera;
	input_ = inputManager;
	dxCommon_ = dxCommon;

	camera_->AddCamera("main2", CameraType::FollowCamera);
	camera_->SetActiveCamera("main2");

	lightManager_ = std::make_unique<LightManager>(dxCommon);
	lightManager_->Initialize();
	for(int i = 0; i < 5; ++i) {
		lightManager_->AddLight(LightType::DIRECTIONALLIGHT);
		lightManager_->SetDirectionalLightIntensity(i, 2.0f);
	}
	lightManager_->SetDirectionalLightDir(1, { 0.0f, 1.0f, 0.0f });
	lightManager_->SetDirectionalLightDir(2, { 1.0f, 0.0f, 0.0f });
	lightManager_->SetDirectionalLightDir(3, { 0.0f, -1.0f, 0.0f });
	lightManager_->SetDirectionalLightDir(4, { -1.0f, 0.0f, 0.0f });

	//オブジェクトたちの初期化
	mapchip_ = std::make_unique<MapChip>();
	mapchip_->Initialize(dxCommon, lightManager_.get());
	mapchip_->LoadMapChipCSV("Resources/map/mapData.csv");

	player_ = std::make_unique<Player>(dxCommon, camera, inputManager, lightManager_.get(), mapchip_.get());
	player_->Initialize();

	sousa_ = std::make_unique<Sprite>(dxCommon);
	sousa_->SetTexture("sousa");
	sousa_->Initialize({ 5.0f, 650.0f, 0.0f });

	skydome_ = std::make_unique<Model>(dxCommon, lightManager_.get());
	skydome_->SetModelData("skydome.obj");
	skydome_->SetTexture("skydome");
	skydome_->Initialize();
	skydome_->IsLighting(LightReflectionModel::None);

	GenerateEnemies();
}

void PlayScene::Update () {
	if(input_->GetRawInput()->Trigger(VK_F1)) {
		nextScene_ = new TitleScene();
		SceneManager::GetInstance()->SetNextScene(nextScene_);
	}
	lightManager_->Update();
	lightManager_->ImGui();

	mapchip_->Update(camera_->GetVPMatrix(), camera_->GetPosition("Debug"));
	//mapchip_->ImGui("mapchip");

	player_->Update();
	player_->ImGui();

	// ★敵を一括更新し、当たり判定もチェックする
	for(auto& enemy : enemies_) {
		enemy->Update();

		if(CheckCollisionAABB(player_->GetAABB(), enemy->GetAABB())) {
			// プレイヤーにダメージを与えるメソッドを呼ぶでやんす！
			player_->OnDamageFromEnemy();
		}

		// プレイヤーの武器との当たり判定
		if(enemy->IsAlive() && player_->GetWeapon()->IsAttacking()) {
			if(player_->GetWeapon()->CheckCollision(enemy->GetAABB())) {
				enemy->OnHit(1, player_->GetTransform().translate);

				//下斬り時のホッピング
				if(player_->GetUpDownDir() < -0.1f) {
					player_->SetVelocity({ 0.0f, 0.2f, 0.0f });
					player_->ResetDoubleJump();
				}
			}
		}
	}

	// 完全に死亡（消滅演出終了）した敵をリストから削除する
	enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(),
								  [](const std::unique_ptr<BaseEnemy>& e) { return e->IsDead(); }), enemies_.end());
	
	camera_->SetFollowTarget("main2", player_->GetTransform());
	camera_->Update();
	camera_->ImGui();

	sousa_->Update();

	// 死亡検知
	if(!player_->IsAlive()) {
		nextScene_ = new GameoverScene();
		SceneManager::GetInstance()->SetNextScene(nextScene_);
	}

	if(player_->IsGoalReached()) {
		// ここで次のシーンへ切り替える処理を呼ぶでやんす！
		nextScene_ = new ClearScene();
		SceneManager::GetInstance()->SetNextScene(nextScene_);
	}

	skydome_->Update(&camera_->GetCameraData());
}
void PlayScene::Draw () {
	skydome_->Draw();
	mapchip_->Draw();
	player_->Draw();

	//敵を一括描画
	for(auto& enemy : enemies_) {
		enemy->Draw();
	}

	sousa_->Draw();
}

void PlayScene::StopToResources() {

}

void PlayScene::GenerateEnemies() {
	// マップチップに溜まったポップデータを元に生成
	for(const auto& data : mapchip_->GetEnemyPopDatas()) {
		std::unique_ptr<BaseEnemy> newEnemy = nullptr;

		if(data.type == MapChipType::kFly) {
			newEnemy = std::make_unique<Fly>(dxCommon_, lightManager_.get(), mapchip_.get());
		}
		else if(data.type == MapChipType::kKorokoro) {
			newEnemy = std::make_unique<Korokoro>(dxCommon_, lightManager_.get(), mapchip_.get());
		}


		if(newEnemy) {
			newEnemy->SetPosition(data.position);
			newEnemy->Initialize();
			enemies_.push_back(std::move(newEnemy));
		}
	}
	// 生成し終わったらデータはクリアしておくでやんす
	mapchip_->ClearEnemyPopDatas();
}