#include "TitleScene.h"
#include "CameraOrganizer.h"
#include "InputManager.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "Easing.h"
#include "imgui.h"
#include "SceneManager.h"
#include "SceneType.h"

//デルタタイムを定義
const float kDeltaTime = 1.0f / 60.0f;

TitleScene::TitleScene() {
	//リソースの準備をする
	TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png", "monsterBall");
}

TitleScene::~TitleScene () {
	//リソースを解放する
	TextureManager::GetInstance()->UnloadTexture("Resources/monsterBall.png");
}

void TitleScene::Initialize (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	sphere_ = std::make_unique<SphereMesh>(dxCommon);
	sphere_->Initialize({0.0f, 0.0f, 0.0f}, 3.0f);

	uint32_t sizeDirectionalLight = (sizeof(DirectionalLight) + 0xFF) & ~0xFF;
	dierctionalLightResource_ = DxCommon::GetInstance()->CreateBufferResource(sizeDirectionalLight);
	//書き込むためのアドレス取得
	dierctionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	//実際に書き込み
	directionalLightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData_->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData_->intensity = 1.0f;
	//ライティング用の変数
	for(int i = 0; i < 4; ++i) {
		colorLight[i] = 1.0f;
	}

	camera_ = camera;
	input_ = inputManager;
}

void TitleScene::Update () {
	if(input_->GetRawInput()->Trigger(VK_F1)) {
		nextScene_ = new PlayScene();
		SceneManager::GetInstance()->SetNextScene(nextScene_);
	}

	camera_->Update();

	sphere_->Update(camera_->GetPosition("Debug"), &camera_->GetVPMatrix());
	sphere_->ImGui();

	//光源のdirectionの正規化
	directionalLightData_->direction = Math::Normalize (directionalLightData_->direction); 

	//ライトのImGui
	if(ImGui::ColorEdit4("light", colorLight)) {
		// 色が変更されたらmaterialDataに反映
		directionalLightData_->color.x = colorLight[0];
		directionalLightData_->color.y = colorLight[1];
		directionalLightData_->color.z = colorLight[2];
		directionalLightData_->color.w = colorLight[3];
	}
	ImGui::DragFloat3("direction", &directionalLightData_->direction.x, 0.01f);
	ImGui::DragFloat("intensity", &directionalLightData_->intensity, 0.01f);
}

void TitleScene::Draw () {
	sphere_->Draw(TextureManager::GetInstance()->GetTextureHandle("monsterBall"), dierctionalLightResource_.Get());
}