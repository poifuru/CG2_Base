#include "TestScene.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "imgui.h"

TestScene::TestScene() {
	ModelManager::GetInstance()->LoadModelData("Resources/AnimatedCube", "AnimatedCube.gltf");
	ModelManager::GetInstance()->LoadAnimationData("Resources/AnimatedCube", "AnimatedCube.gltf");
	TextureManager::GetInstance()->TextureManager::LoadTexture(
		"Resources/AnimatedCube/AnimatedCube_BaseColor.png", "Cube"
	);
	// 人のアニメーション
	ModelManager::GetInstance()->LoadModelData("Resources/human", "walk.gltf");
	ModelManager::GetInstance()->LoadAnimationData("Resources/human", "walk.gltf");
	TextureManager::GetInstance()->LoadTexture("Resources/human/white.png", "white");

	ModelManager::GetInstance()->LoadModelData("Resources/player", "player.obj");
	TextureManager::GetInstance()->LoadTexture("Resources/player/player.png", "player");

	TextureManager::GetInstance()->LoadTexture("Resources/Skybox/test2.dds", "skybox");
}

TestScene::~TestScene() {

}

void TestScene::Initialize(CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) {
	camera_ = camera;
	input_ = inputManager;

	lightManager_ = std::make_unique<LightManager>(dxCommon);
	lightManager_->Initialize();
	lightManager_->AddLight(LightType::POINTLIGHT);
	lightManager_->AddLight(LightType::DIRECTIONALLIGHT);

	modelFactory_ = ModelFactory::GetInstance();
	modelFactory_->SetLightManager(lightManager_.get());

	cube_ = std::move(modelFactory_->CreateModel("AnimatedCube.gltf", "Cube"));
	cube_->SetAnimation(ModelManager::GetInstance()->GetAnimationData("AnimatedCube.gltf").lock().get());

	skybox_ = std::make_unique<Skybox>(dxCommon);
	skybox_->Initialize("skybox");
	skybox_->SetRenderType(RenderType::Skybox);

	/*human_ = std::make_unique<Model>(dxCommon, lightManager_.get());
	human_->SetModelData("walk.gltf");
	human_->SetTexture("white");
	human_->SetAnimation("walk.gltf");
	human_->Initialize({ 100.0f, 100.0f, 100.0f }, { Math::Deg2Rad(-90.0f), Math::Deg2Rad(180.0f), 0.0f }, {});
	human_->SkeletonInit();*/

	RenderSystem::GetInstance()->SetLightBuffer(lightManager_->GetLightGPUAddress());
	RenderSystem::GetInstance()->SetCameraBuffer(camera_->GetCameraGPUAddress());
	RenderSystem::GetInstance()->SetSkyboxBuffer(skybox_->GetTextureHandle());
}

void TestScene::Update() {
	camera_->Update();

	lightManager_->Update();
	lightManager_->ImGui();

	skybox_->Update(&camera_->GetCameraData());
	cube_->Update(&camera_->GetCameraData());

	/*human_->Update(&camera_->GetCameraData());
	human_->ImGui("human_walk");*/

	camera_->ImGui();
	cube_->ImGui("AnimationCube");
	ImGui::ShowDemoWindow();
}

void TestScene::Draw() {
	skybox_->Draw();
	cube_->Draw();
	/*human_->Draw();*/
}

void TestScene::StopToResources() {

}