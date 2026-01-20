#pragma once
#include "Scene.h"
#include <memory>
#include "Sprite.h"
#include "Model.h"
#include "MeshParticle.h"
#include "Mesh.h"
#include "SphereMesh.h"
#include "LightManager.h"

class TitleScene : public Scene {
public:		//メンバ関数
	TitleScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon);
	~TitleScene () override;

	void Initialize () override;
	void Update () override;
	void Draw () override;

private:	//メンバ変数
	std::unique_ptr<LightManager> lightManager_ = nullptr;
	std::unique_ptr<SphereMesh> sphere_ = nullptr;
	std::unique_ptr<Model> terrain_ = nullptr;
};