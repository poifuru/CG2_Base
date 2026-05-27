#include "PlayScene.h"
#include "MathFunction.h"
#include "CameraOrganizer.h"
#include "InputManager.h"
#include "ModelFactory.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "TextureManager.h"

PlayScene::PlayScene () {
	TextureManager::GetInstance()->LoadTexture("Resources/Skybox/test2.dds");
}

PlayScene::~PlayScene () {
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

	ModelFactory::GetInstance()->SetLightManager(lightManager_.get());

	//オブジェクトたちの初期化
	/*player_ = std::make_unique<Player>(dxCommon_, camera_, input_, lightManager_.get());
	player_->Initialize();*/

	/*enemyManager_ = std::make_unique<EnemyManager>();
	enemyManager_->Initialize(dxCommon, lightManager_.get(), camera);*/

	skybox_ = std::make_unique<Skybox>(dxCommon);
	skybox_->Initialize("Resources/Skybox/test2.dds");

	RenderSystem::GetInstance()->SetCameraBuffer(camera_->GetCameraGPUAddress());
	RenderSystem::GetInstance()->SetLightBuffer(lightManager_->GetLightGPUAddress());
	RenderSystem::GetInstance()->SetSkyboxBuffer(skybox_->GetTextureHandle());
}

void PlayScene::Update () {
	if(input_->GetRawInput()->Trigger(VK_F1)) {
		//nextScene_ = std::make_unique<TitleScene>();
		SceneManager::GetInstance()->SetNextScene(std::move(nextScene_));
	}
	lightManager_->Update();
	lightManager_->ImGui();

	/*camera_->SetFollowTarget("main2", player_->GetTransform());*/
	camera_->Update();
	camera_->ImGui();

	/*player_->Update();
	player_->ImGui();*/

	//enemyManager_->Update(player_->GetTransform().translate.z);

	//// --- 弾と敵の当たり判定 ---
	//for (auto& bullet : player_->GetBullets()) {
	//	if (!bullet->IsActive()) continue;
	//	for (auto& enemy : enemyManager_->GetEnemies()) {
	//		if (!enemy || !enemy->IsActive()) continue;
	//		if (Math::IsCollision(bullet->GetAABB(), enemy->GetAABB())) {
	//			bullet->SetIsActive(false);  // 弾を消す
	//			enemy->SetIsActive(false);   // 敵を消す
	//			break;
	//		}
	//	}
	//}

	skybox_->Update(&camera_->GetCameraData());
}

void PlayScene::Draw () {
	skybox_->Draw();

	//player_->Draw();
	//enemyManager_->Draw();
}

void PlayScene::StopToResources() {

}