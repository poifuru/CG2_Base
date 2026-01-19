#pragma once
#include "BaseScene.h"
#include <memory>
#include "Sprite.h"
#include "Model.h"

class GameoverScene : public BaseScene {
public:		//メンバ関数
	GameoverScene ();
	~GameoverScene () override;

	void Initialize (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) override;
	void Update () override;
	void Draw () override;
	
private:	//メンバ変数
};