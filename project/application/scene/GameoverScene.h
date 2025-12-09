#pragma once
#include "Scene.h"
#include <memory>
#include "Sprite.h"
#include "Model.h"

class GameoverScene : public Scene {
public:		//メンバ関数
	GameoverScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon);
	~GameoverScene () override;

	void Initialize () override;
	void Update () override;
	void Draw () override;

private:	//メンバ変数
	std::unique_ptr<Model> ground_ = nullptr;
	std::unique_ptr<Model> mountain_ = nullptr;
	std::unique_ptr<Model> stone_ = nullptr;
	std::unique_ptr<Model> skydome_ = nullptr;
};