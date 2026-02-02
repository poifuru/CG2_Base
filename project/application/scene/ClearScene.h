#pragma once
#include "BaseScene.h"
#include <memory>
#include "Sprite.h"
#include "Model.h"

class ClearScene : public BaseScene {
public:		//メンバ関数
	ClearScene ();
	~ClearScene () override;

	void Initialize (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) override;
	void Update () override;
	void Draw () override;
	void StopToResources() override;

private:	//メンバ変数
};