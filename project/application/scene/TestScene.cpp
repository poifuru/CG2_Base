#include "TestScene.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "imgui.h"

TestScene::TestScene() {
	ModelManager::GetInstance()->LoadModelData("Resources/AnimatedCube", "AnimatedCube.gltf");
	ModelManager::GetInstance()->LoadAnimationData("Resources/AnimatedCube", "AnimatedCube.gltf");
	TextureManager::GetInstance()->TextureManager::LoadTexture(
		"Resources/AnimatedCube/AnimatedCube_BaseColor.png"
	);
	// 人のアニメーション
	ModelManager::GetInstance()->LoadModelData("Resources/human", "walk.gltf");
	ModelManager::GetInstance()->LoadAnimationData("Resources/human", "walk.gltf");
	TextureManager::GetInstance()->LoadTexture("Resources/human/white.png");

	ModelManager::GetInstance()->LoadModelData("Resources/player", "player.obj");
	TextureManager::GetInstance()->LoadTexture("Resources/player/player.png");

	// パーティクルのテクスチャ
	TextureManager::GetInstance()->LoadTexture("Resources/Particle/circle2.png");
	TextureManager::GetInstance()->LoadTexture("Resources/Particle/gradationLine.png");

	TextureManager::GetInstance()->LoadTexture("Resources/Skybox/test2.dds");
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

	cube_ = std::move(modelFactory_->CreateModel("AnimatedCube.gltf", "Resources/AnimatedCube/AnimatedCube_BaseColor.png"));
	cubeAnimator_ = std::make_unique<Animator>(dxCommon);
	cubeAnimator_->Initialize(cube_.get());
	cubeAnimator_->BindAnimation(ModelManager::GetInstance()->GetAnimationData("AnimatedCube.gltf").lock().get());
	cube_->SetAnimator(cubeAnimator_.get());
	cube_->SetTranslate(Vector3{ -5.0f, 0.0f, 0.0f });

	human_ = std::move(modelFactory_->CreateModel("walk.gltf", "Resources/human/white.png"));
	humanAnimator_ = std::make_unique<Animator>(dxCommon);
	humanAnimator_->Initialize(human_.get());
	humanAnimator_->BindAnimation(ModelManager::GetInstance()->GetAnimationData("walk.gltf").lock().get());
	human_->SetAnimator(humanAnimator_.get());
	human_->SetRenderType(RenderType::Skining);
	human_->SetRotate(Vector3{ 0.0f, Math::Deg2Rad(180.0f), 0.0f });
	human_->SetTranslate(Vector3{ 5.0f, 0.0f, 0.0f });

	parSystem_ = std::make_unique<ParticleSystem>(dxCommon);
	parSystem_->Initialize();

	skybox_ = std::make_unique<Skybox>(dxCommon);
	skybox_->Initialize("Resources/Skybox/test2.dds");
	skybox_->SetRenderType(RenderType::Skybox);

	RenderSystem::GetInstance()->SetLightBuffer(lightManager_->GetLightGPUAddress());
	RenderSystem::GetInstance()->SetCameraBuffer(camera_->GetCameraGPUAddress());
	RenderSystem::GetInstance()->SetSkyboxBuffer(skybox_->GetTextureHandle());
}

void TestScene::Update() {
	camera_->Update();

	lightManager_->Update();

	cube_->Update(&camera_->GetCameraData());
	human_->Update(&camera_->GetCameraData());

	parSystem_->Update(camera_->GetCameraData());

	skybox_->Update(&camera_->GetCameraData());

	ImGui();
}

void TestScene::Draw() {
	skybox_->Draw();
	cube_->Draw();
	human_->Draw();
	parSystem_->Draw();
}

void TestScene::StopToResources() {

}

void TestScene::ImGui() {
#ifdef USEIMGUI
	ImGui::Begin("TestScene");
	if(ImGui::BeginTabBar("Setting")) {
		if(ImGui::BeginTabItem("Camera")) {
			camera_->ImGui();
			ImGui::EndTabItem();
		}

		if(ImGui::BeginTabItem("Light")) {
			ImGui::Spacing();
			lightManager_->ImGui();
			ImGui::EndTabItem();
		}

		if(ImGui::BeginTabItem("Objects")) {
			cube_->ImGui("AnimationCube");
			human_->ImGui("AnimationHuman");
			ImGui::EndTabItem();
		}

		if(ImGui::BeginTabItem("ParticleSystem")) {
			parSystem_->ImGui();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
#endif
}
