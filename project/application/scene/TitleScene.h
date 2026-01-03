#pragma once
#include "Scene.h"
#include <memory>
#include "Sprite.h"
#include "Model.h"
#include "MeshParticle.h"
#include "Mesh.h"

class TitleScene : public Scene {
public:		//メンバ関数
	TitleScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon);
	~TitleScene () override;

	void Initialize () override;
	void Update () override;
	void Draw () override;

private:	//メンバ変数
	std::unique_ptr<Model> model_ = nullptr;
};