#include "PlayScene.h"
#include "MathFunction.h"
#include "CameraOrganizer.h"
#include "InputManager.h"

PlayScene::PlayScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	player_ = std::make_unique<Player>(inputManager, dxCommon);
	boss_ = std::make_unique<Boss>(dxCommon, player_.get());

	//地面のモデル
	ground_ = std::make_unique<Model> (dxCommon);

	//火山
	mountain_ = std::make_unique<Model> (dxCommon);

	//岩の壁
	stone_ = std::make_unique<Model> (dxCommon);

	//岩の壁
	stone2_ = std::make_unique<Model> (dxCommon);
	ModelManager::GetInstance ()->LoadModelData ("Resources/field/stone2", "stone2");

	//岩の壁
	stone3_ = std::make_unique<Model> (dxCommon);
	ModelManager::GetInstance ()->LoadModelData ("Resources/field/stone3", "stone3");

	//天球
	skydome_ = std::make_unique<Model> (dxCommon);

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
	player_->ResetData();
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

	//岩2
	stone2_->SetModelData ("stone2");
	stone2_->SetTexture ("stone2");
	stone2_->Initialize ({ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.2f, 0.0f });
	stone2_->IsLighting (LightReflectionModel::HalfLambert);
	//岩の色
	Vector4 stoneColor2 = { 0.33f, 0.33f, 0.33f, 0.2f };
	stone2_->SetColor (stoneColor2);

	//岩2
	stone3_->SetModelData ("stone3");
	stone3_->SetTexture ("stone3");
	stone3_->Initialize ({ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.2f, 0.0f });
	stone3_->IsLighting (LightReflectionModel::HalfLambert);
	//岩の色
	Vector4 stoneColor3 = { 0.33f, 0.33f, 0.33f, 1.0f };
	stone3_->SetColor (stoneColor3);

	//天球
	skydome_->SetModelData ("skydome");
	skydome_->SetTexture ("skydome");
	skydome_->Initialize ();
	skydome_->IsLighting (LightReflectionModel::HalfLambert);

	camera_->AddCamera ("FollowCamera", CameraType::FollowCamera);
	camera_->SetActiveCamera ("FollowCamera");
	camera_->SetFollowTarget ("FollowCamera", player_->GetTransform ());

	enemies_->Initialize(player_.get(), boss_.get());
}

void PlayScene::Update () {
	boss_->ImGuiControl();
	player_->Update(camera_->GetVPMatrix());
	boss_->Update(camera_->GetVPMatrix());
	ground_->Update (camera_->GetVPMatrix ());
	mountain_->Update (camera_->GetVPMatrix ());
	stone_->Update (camera_->GetVPMatrix ());
	stone2_->Update (camera_->GetVPMatrix ());
	stone3_->Update (camera_->GetVPMatrix ());
	skydome_->Update (camera_->GetVPMatrix ());
	enemies_->SetPos ({ boss_->GetPosition ().x,0.0f,boss_->GetPosition ().z });
	enemies_->Update(camera_->GetVPMatrix ());

	// [ 当たり判定 ]
	collisionManager_->Begin();
	collisionManager_->SetColliders(&player_->GetAttackCollider());
	collisionManager_->SetColliders(&player_->GetPlayerBodyCollider());
	for (const auto& enemy : enemies_->GetEnemies()) {
		collisionManager_->SetColliders(&enemy->GetAttackCollider());
		collisionManager_->SetColliders(&enemy->GetBodyCollider());
	}
	for (Collider* attackCollider : boss_->GetAttackColliders()) {
		collisionManager_->SetColliders(attackCollider);
	}
	collisionManager_->SetColliders(boss_->GetBodyCollider());
	collisionManager_->CheckAllCollisions();

	camera_->Update ();

	if (player_->GetIsDead()) {
		nextScene_ = SceneLabel::Gameover;
		isFinish_ = true;
	}

	if (boss_->GetClear()) {
		nextScene_ = SceneLabel::Clear;
		isFinish_ = true;
	}
}

void PlayScene::Draw () {
	skydome_->Draw ();
	ground_->Draw ();
	mountain_->Draw ();
	stone_->Draw ();
	player_->Draw ();
	boss_->Draw ();
	enemies_->Draw ();
	stone3_->Draw ();
	stone2_->Draw ();
}