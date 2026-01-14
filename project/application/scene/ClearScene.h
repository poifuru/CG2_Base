#pragma once
#include "Scene.h"
#include <memory>
#include "Sprite.h"
#include "Model.h"

class ClearScene : public Scene {
public:		//メンバ関数
	ClearScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon);
	~ClearScene () override;

	void Initialize () override;
	void Update () override;
	void Draw () override;

private:	//メンバ変数
};