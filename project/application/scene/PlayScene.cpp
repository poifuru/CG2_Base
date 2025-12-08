#include "PlayScene.h"
#include "MagosuyaEngine.h"
#include "MathFunction.h"

PlayScene::PlayScene (MagosuyaEngine* magosuya) {
	magosuya_ = magosuya;
	camera_ = std::make_unique<CameraData>();
	player_ = std::make_unique<Player>(magosuya);
	boss_ = std::make_unique<Boss>(magosuya_);
	enemies_ = std::make_unique<EnemyManager>(magosuya);
	collisionManager_ = std::make_unique<CollisionManager>();
}

PlayScene::~PlayScene () {
	
}

void PlayScene::Initialize () {
	nowScene_ = SceneLabel::Play;
	isFinish_ = false;
	camera_ = std::make_unique<CameraData>();
	camera_->transform = {
		{1.0f, 1.0f, 1.0f},
		{0.6f, 0.0f, 0.0f},
		{0.0f, 40.0f, -60.0f},
	};

	player_->Initialize();
	boss_->Initialize();
	enemies_->Initialize(player_.get());
}

void PlayScene::Update () {
	boss_->ImGuiControl();

	//カメラ
	camera_->world = MakeAffineMatrix(camera_->transform.scale, camera_->transform.rotate, camera_->transform.translate);
	camera_->view = Inverse(camera_->world);
	camera_->proj = MakePerspectiveFOVMatrix(
		0.45f, float(magosuya_->GetDxCommon()->GetWinAPI()->kClientWidth) / float(magosuya_->GetDxCommon()->GetWinAPI()->kClientHeight), 0.1f, 100.0f
	);
	Matrix4x4 vp = Multiply(camera_->view, camera_->proj);

	//if (magosuya_->GetRawInput()->Trigger(VK_SPACE)) {
	//	nextScene_ = SceneLabel::Title;
	//	isFinish_ = true;
	//}

	player_->Update(&vp);
	boss_->Update(&vp);
	enemies_->Update(&vp);

	// [ 当たり判定 ]
	collisionManager_->Begin();
	collisionManager_->SetColliders(&player_->GetAttackCollider());
	collisionManager_->SetColliders(&player_->GetPlayerBodyCollider());
	for (const auto& enemy : enemies_->GetEnemies()) {
		collisionManager_->SetColliders(&enemy->GetAttackCollider());
		collisionManager_->SetColliders(&enemy->GetBodyCollider());
	}
	collisionManager_->CheckAllCollisions();
}

void PlayScene::Draw () {
	player_->Draw();
	boss_->Draw();
	enemies_->Draw();
}