#pragma once
#include "BaseScene.h"
#include "LightManager.h"
#include "Model.h"
#include "Sprite.h"
#include "Skybox.h"
#include "ModelFactory.h"

class TestScene : public BaseScene {
public:		//メンバ関数
	TestScene();
	~TestScene() override;

	void Initialize(CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) override;
	void Update() override;
	void Draw() override;
	void StopToResources() override;

private:	//メンバ変数
	std::unique_ptr<LightManager> lightManager_ = nullptr;
	ModelFactory* modelFactory_ = nullptr;

	std::unique_ptr<Model> cube_ = nullptr;

	// スカイボックス
	std::unique_ptr<Skybox> skybox_ = nullptr;
};