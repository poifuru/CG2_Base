#include "PlayScene.h"
#include "MathFunction.h"
#include "CameraOrganizer.h"
#include "InputManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "PostEffectManager.h"
#include <algorithm>

PlayScene::PlayScene () {
	ModelManager::GetInstance()->LoadModelData("Resources/plane", "plane.obj");
	TextureManager::GetInstance()->LoadTexture("Resources/plane/plane.png", "plane");

	ModelManager::GetInstance()->LoadModelData("Resources/stage", "stage.obj");
	TextureManager::GetInstance()->LoadTexture("Resources/stage/stage.png", "stage");
}

PlayScene::~PlayScene () {
}

void PlayScene::Initialize (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	camera_ = camera;
	input_ = inputManager;
	dxCommon_ = dxCommon;

	// 固定カメラを使用して、Update内で手動でレールに追従させる
	camera_->AddCamera("main2", CameraType::FixedPointCamera);
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

	// TPS化: レールパスの初期化は行わない
	railPath_ = nullptr;

	//オブジェクトたちの初期化
	player_ = std::make_unique<Player>(dxCommon_, camera_, input_, lightManager_.get());
	player_->Initialize();
	player_->SetRail(nullptr); // プレイヤーへのレール設定を解除

	enemyManager_ = std::make_unique<EnemyManager>();
	enemyManager_->Initialize(dxCommon, lightManager_.get(), camera);
	player_->SetEnemyManager(enemyManager_.get());

	skybox_ = std::make_unique<Skybox>(dxCommon);
	skybox_->Initialize("Resources/Skybox/test2.dds", "skybox");

	waterSurface_ = std::make_unique<Model>(dxCommon, lightManager_.get());
	waterSurface_->SetModelData("plane.obj");
	waterSurface_->SetTexture("plane");
	waterSurface_->Initialize({ 1000.0f, 1000.0f, 1.0f }, { Math::Deg2Rad(90.0f), 0.0f, 0.0f }, {});
	waterSurface_->SetMetallic(0.0f);
	waterSurface_->SetEnvironmentCoefficient(0.0f);

	stage_ = std::make_unique<Model>(dxCommon, lightManager_.get());
	stage_->SetModelData("stage.obj");
	stage_->SetTexture("stage");
	stage_->Initialize({ 0.5f, 1.0f, 1.0f }, { 0.0f, 0.0f, Math::Deg2Rad(-90.0f) }, {0.0f, -50.0f, 0.0f});
	stage_->SetMetallic(0.0f);
	stage_->SetEnvironmentCoefficient(0.0f);
	stage_->SetColor({ 0.492f, 0.238f, 0.0f, 1.0f });

	// ポストエフェクトの状態を初期化
	PostEffectManager::GetInstance()->ClearEffects();
	PostEffectManager::GetInstance()->SetEffectActive(PostEffectType::Fog, true);
	PostEffectManager::GetInstance()->SetEffectActive(PostEffectType::Vignette, true);

	// マリンスノー用テクスチャのロードとパーティクル初期化
	TextureManager::GetInstance()->LoadTexture("Resources/circle2.png", "circle");
	particle_ = std::make_unique<Particle>(dxCommon_);
	particle_->Initialize();
	particle_->SetTexHandle(TextureManager::GetInstance()->GetTextureHandle("circle"));
	particle_->SetMarineSnow(true);
}

void PlayScene::Update () {
	if(input_->GetRawInput()->Trigger(VK_F1)) {
		nextScene_ = std::make_unique<TitleScene>();
		SceneManager::GetInstance()->SetNextScene(std::move(nextScene_));
	}
	lightManager_->Update();
	lightManager_->ImGui();

	// TPSカメラの更新 (アクティブカメラが "main2" の場合のみ)
	if (camera_->GetActiveCameraName() == "main2") {
		// カメラの回転角度（Yaw/Pitch）を管理
		static Vector3 cameraRotate = { Math::Deg2Rad(10.0f), 0.0f, 0.0f };

		// マウスの右クリックドラッグ（PushMouse(1)）でカメラを回転させる
		if (input_->GetRawInput()->PushMouse(1)) {
			cameraRotate.y += (float)input_->GetRawInput()->GetMouseDeltaX() * 0.002f;
			cameraRotate.x += (float)input_->GetRawInput()->GetMouseDeltaY() * 0.002f;

			// 縦の回転（Pitch）を -60度〜60度にクランプ
			cameraRotate.x = std::clamp(cameraRotate.x, Math::Deg2Rad(-60.0f), Math::Deg2Rad(60.0f));
		}

		// プレイヤーの位置を取得
		Vector3 playerPos = player_->GetTransform().translate;

		// 注視点はプレイヤーの少し上 (Y+2.0f)
		Vector3 lookAtTarget = Math::Add(playerPos, { 0.0f, 2.0f, 0.0f });

		// カメラの回転行列を作成
		Matrix4x4 camRotMat = Math::MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, cameraRotate, { 0.0f, 0.0f, 0.0f });

		// カメラの位置を注視点から後ろへ25mオフセット
		Vector3 cameraOffset = { 0.0f, 0.0f, -25.0f };
		Vector3 rotatedOffset = Math::Transform(cameraOffset, camRotMat);
		Vector3 cameraPos = Math::Add(lookAtTarget, rotatedOffset);

		camera_->SetPosition(cameraPos);
		camera_->SetRotate(cameraRotate);
	}

	camera_->Update();
	camera_->ImGui();

	player_->Update();
	player_->ImGui();

	enemyManager_->Update(player_->GetTransform().translate, railPath_.get());

	// --- 弾と敵の当たり判定 ---
	for (auto& bullet : player_->GetBullets()) {
		if (!bullet->IsActive()) continue;
		for (auto& enemy : enemyManager_->GetEnemies()) {
			if (!enemy || !enemy->IsActive()) continue;
			if (Math::IsCollision(bullet->GetAABB(), enemy->GetAABB())) {
				bullet->SetIsActive(false);  // 弾を消す
				enemy->SetIsActive(false);   // 敵を消す
				break;
			}
		}
	}

	skybox_->Update(&camera_->GetCameraData());
	waterSurface_->Update(&camera_->GetCameraData());
	waterSurface_->ImGui("waterSurface");

	stage_->Update(&camera_->GetCameraData());
	stage_->ImGui("stage");

	// パーティクルの更新とデバッグUI
	if (particle_) {
		particle_->Update(&camera_->GetCameraData().world, &camera_->GetVPMatrix());
		particle_->ImGui();
	}

	PostEffectManager::GetInstance()->ImGui();
}

void PlayScene::Draw () {
	if(!camera_->InWater()) {
		skybox_->Draw();
		PostEffectManager::GetInstance()->SetEffectActive(PostEffectType::Fog, false);
		PostEffectManager::GetInstance()->SetEffectActive(PostEffectType::Vignette, false);
	}
	else {
		PostEffectManager::GetInstance()->SetEffectActive(PostEffectType::Fog, true);
		PostEffectManager::GetInstance()->SetEffectActive(PostEffectType::Vignette, true);
	}

	if (railPath_) {
		railPath_->Draw(camera_->GetVPMatrix(), { 0.0f, 0.0f, 0.0f, 1.0f });
	}

	
	waterSurface_->Draw();
	stage_->Draw();
	player_->Draw();
	enemyManager_->Draw();
}

void PlayScene::DrawUI() {
	// ポストエフェクト適用後にマリンスノーを描画 (フォグに飲まれないようにするため)
	if (particle_) {
		particle_->Draw();
	}

	if (player_) {
		player_->DrawUI();
	}
}

void PlayScene::StopToResources() {

}