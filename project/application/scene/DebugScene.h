//#pragma once
//#include "BaseScene.h"
//#include <memory>
//#include "Sprite.h"
//#include "Model.h"
//#include "MeshParticle.h"
//#include "Primitive.h"
//#include "SphereMesh.h"
//#include "LightManager.h"
//
//class DebugScene : public BaseScene {
//public:		//メンバ関数
//	DebugScene();
//	~DebugScene() override;
//
//	void Initialize(CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) override;
//	void Update() override;
//	void Draw() override;
//	void StopToResources() override;
//
//private:	//メンバ変数
//	std::unique_ptr<LightManager> lightManager_ = nullptr;
//	std::unique_ptr<SphereMesh> sphere_ = nullptr;
//	std::unique_ptr<Model> terrain_ = nullptr;
//	std::unique_ptr<Model> plane1_ = nullptr;
//	std::unique_ptr<Model> plane2_ = nullptr;
//	uint32_t bgm = 0;
//};
//
