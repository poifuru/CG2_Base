#include "PlayScene.h"
#include "MathFunction.h"
#include "CameraOrganizer.h"
#include "InputManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "PostEffectManager.h"

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

	// レールパスの初期化 (テスト用のS字/カーブルート)
	railPath_ = std::make_unique<RailPath>();
	std::vector<Vector3> controlPoints = {
		{ 0.0f, 0.0f, -20.0f },  // 補助点 (曲線の入り口用)
		{ 0.0f, -10.0f, 0.0f },    // 始点
		{ 0.0f, -10.0f, 100.0f },
		{ 50.0f, -30.0f, 200.0f }, // 右へゆるやかにカーブする
		{ 100.0f, -10.0f, 300.0f },
		{ 100.0f, -10.0f, 500.0f }, // 終点
		{ 100.0f, 0.0f, 520.0f }  // 補助点 (曲線の出口用)
	};
	railPath_->Initialize(controlPoints, 0.0005f); // 進む速さ (フレームごとの進行率)

	//オブジェクトたちの初期化
	player_ = std::make_unique<Player>(dxCommon_, camera_, input_, lightManager_.get());
	player_->Initialize();
	player_->SetRail(railPath_.get()); // プレイヤーにレールを設定

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
	stage_->Initialize({ 0.5f, 1.0f, 1.0f }, { 0.0f, 0.0f, Math::Deg2Rad(-90.0f) }, {0.0f, -40.0f, 0.0f});
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

	// レールの更新
	if (railPath_) {
		railPath_->Update();
	}

	// カメラをレールに追従させる (アクティブカメラが "main2" の場合のみ)
	if (railPath_ && camera_->GetActiveCameraName() == "main2") {
		Vector3 railPos = railPath_->GetPosition();
		Matrix4x4 railRot = railPath_->GetRotationMatrix();

		// カメラのレールに対するオフセット (少し後ろ・少し上)
		Vector3 cameraOffset = { 0.0f, 0.0f, -50.0f };
		Vector3 rotatedOffset = Math::Transform(cameraOffset, railRot);
		Vector3 cameraPos = Math::Add(railPos, rotatedOffset);

		camera_->SetPosition(cameraPos);

		// レールの進行方向（前方向）を向くように回転を設定する
		Vector3 direction = { railRot.m[2][0], railRot.m[2][1], railRot.m[2][2] };
		Vector3 cameraRotate = { 0.0f, 0.0f, 0.0f };
		if (Math::Length(direction) > 0.001f) {
			cameraRotate.y = std::atan2(direction.x, direction.z);
			float xzLength = std::sqrt(direction.x * direction.x + direction.z * direction.z);
			cameraRotate.x = std::atan2(-direction.y, xzLength);
		}
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