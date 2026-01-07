#include "PlayScene.h"
#include "MathFunction.h"
#include "CameraOrganizer.h"
#include "InputManager.h"
#include "TextureManager.h"
#include "ModelManager.h"

PlayScene::PlayScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	camera_ = camera;
	input_ = inputManager;

	map_ = std::make_unique<Model>(dxCommon);
	ModelManager::GetInstance()->LoadModelData("Resources/map", "map");

	player_ = std::make_unique<Player>(dxCommon, camera);
	boss_ = std::make_unique<Boss>(dxCommon, camera);
}

PlayScene::~PlayScene () {

}

void PlayScene::Initialize () {
	nowScene_ = SceneLabel::Play;
	isFinish_ = false;

	map_->SetModelData("map");
	map_->SetTexture("map");
	map_->Initialize();
	map_->IsLighting(LightReflectionModel::HalfLambert);

	dierctionalLightResource_ = DxCommon::GetInstance()->CreateBufferResource(sizeof(DirectionalLight));
	//書き込むためのアドレス取得
	dierctionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	//実際に書き込み
	directionalLightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData_->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData_->intensity = 1.0f;
	//ライティング用の変数
	colorLight = { 1.0f, 1.0f, 1.0f, 1.0f };

	player_->Initialize();
	player_->SetLight(dierctionalLightResource_.Get());

	boss_->Initialize();
	boss_->SetLight(dierctionalLightResource_.Get());
}

void PlayScene::Update () {
	if(input_->GetRawInput()->Trigger(VK_F1)) {
		nextScene_ = SceneLabel::Title;
		isFinish_ = true;
	}
	camera_->Update();

	map_->Update(&camera_->GetVPMatrix());

	// ボスにプレイヤーの場所を教える（これがないと追跡できないでやんす！）
	boss_->SetTarget(player_->GetTransform());

	player_->Update();
	boss_->Update();

	// 当たり判定を毎フレーム実行するでやんす！
	Collision();

	directionalLightData_->direction = Math::Normalize(directionalLightData_->direction);

	if(player_->GetHp() == 0) {
		nextScene_ = SceneLabel::Gameover;
		isFinish_ = true;
	}
	if(boss_->GetHp() == 0) {
		nextScene_ = SceneLabel::Clear;
		isFinish_ = true;
	}
}

void PlayScene::Draw () {
	map_->Draw(dierctionalLightResource_.Get());
	player_->Draw();
	boss_->Draw();
}

void PlayScene::Collision() {
	//プレイヤーと敵の当たり判定
	if(Math::IsCollision(player_->GetAABBModel(), boss_->GetAABBModel())) {
		player_->IsHit();
	}

	//武器と敵のあたり判定
	if(Math::IsCollision(player_->GetAABBWeapon_(), boss_->GetAABBModel()) && player_->GetIsAttack()) {
		if(!player_->GetAttackIsHit()) {
			boss_->IsHit(true);
			player_->SetAttackIsHit(true);	
		}
	}
}