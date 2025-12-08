#include "PlayScene.h"
#include "MathFunction.h"
#include "CameraOrganizer.h"
#include "InputManager.h"

PlayScene::PlayScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	player_ = std::make_unique<Player>(inputManager, dxCommon);
	boss_ = std::make_unique<Boss>(dxCommon, player_.get());

	//地面のモデル
	ground_ = std::make_unique<Model> (dxCommon);
	ModelManager::GetInstance ()->LoadModelData ("Resources/field/ground", "ground");

	//火山
	mountain_ = std::make_unique<Model> (dxCommon);
	ModelManager::GetInstance ()->LoadModelData ("Resources/field/mountain", "mountain");

	//岩の壁
	stone_ = std::make_unique<Model> (dxCommon);
	ModelManager::GetInstance ()->LoadModelData ("Resources/field/stone", "stone");

	//天球
	skydome_ = std::make_unique<Model> (dxCommon);
	ModelManager::GetInstance ()->LoadModelData ("Resources/skydome", "skydome");

	camera_ = camera;
	input_ = inputManager;
	enemies_ = std::make_unique<EnemyManager>(dxCommon);
	collisionManager_ = std::make_unique<CollisionManager>();
}

PlayScene::~PlayScene () {
	
}

void PlayScene::Initialize () {
	nowScene_ = SceneLabel::Play;
	isFinish_ = false;
	player_->Initialize();
	boss_->Initialize();

	//地面のモデル
	ground_->SetModelData ("ground");
	ground_->SetTexture ("ground");
	ground_->Initialize ({ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
	ground_->IsLighting (LightReflectionModel::HalfLambert);

	//火山
	mountain_->SetModelData ("mountain");
	mountain_->SetTexture ("mountain");
	mountain_->Initialize ({ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.2f, 0.0f });
	mountain_->IsLighting (LightReflectionModel::HalfLambert);

	//岩
	stone_->SetModelData ("stone");
	stone_->SetTexture ("stone");
	stone_->Initialize ({ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.2f, 0.0f });
	stone_->IsLighting (LightReflectionModel::HalfLambert);
	//岩の色
	Vector4 stoneColor = { 0.33f, 0.33f, 0.33f, 1.0f };
	stone_->SetColor (stoneColor);

	//天球
	skydome_->SetModelData ("skydome");
	skydome_->SetTexture ("skydome");
	skydome_->Initialize ();
	skydome_->IsLighting (LightReflectionModel::HalfLambert);

	Transform transform = {};
	camera_->AddCamera ("FollowCamera", CameraType::FollowCamera, transform);
	camera_->SetActiveCamera ("FollowCamera");
	camera_->SetFollowTarget ("FollowCamera", player_->GetTransform ());
	enemies_->Initialize(player_.get());
}

void PlayScene::Update () {
	boss_->ImGuiControl();

	if (input_->GetRawInput()->Trigger(VK_F1)) {
		nextScene_ = SceneLabel::Title;
		isFinish_ = true;
	}

	player_->Update(camera_->GetVPMatrix());
	boss_->Update(camera_->GetVPMatrix());
	ground_->Update (camera_->GetVPMatrix ());
	mountain_->Update (camera_->GetVPMatrix ());
	stone_->Update (camera_->GetVPMatrix ());
	skydome_->Update (camera_->GetVPMatrix ());
	enemies_->Update(camera_->GetVPMatrix ());

	// [ 当たり判定 ]
	collisionManager_->Begin();
	collisionManager_->SetColliders(&player_->GetAttackCollider());
	collisionManager_->SetColliders(&player_->GetPlayerBodyCollider());
	for (const auto& enemy : enemies_->GetEnemies()) {
		collisionManager_->SetColliders(&enemy->GetAttackCollider());
		collisionManager_->SetColliders(&enemy->GetBodyCollider());
	}
	collisionManager_->CheckAllCollisions();
	camera_->Update ();
}

void PlayScene::Draw () {
	player_->Draw();
	boss_->Draw();
	ground_->Draw ();
	mountain_->Draw ();
	stone_->Draw ();
	skydome_->Draw ();
	enemies_->Draw();
}