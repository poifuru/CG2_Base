#pragma once
#include "BaseScene.h"
#include <memory>
#include "Sprite.h"
#include "Model.h"
#include "mapchip.h"
#include "mapChipRenderer.h"
#include "Player.h"
#include "Boss.h"

class PlayScene : public BaseScene {
public:		//メンバ関数
	PlayScene ();
	~PlayScene () override;

	void Initialize (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) override;
	void Update () override;
	void Draw () override;

private:	//メンバ変数
	
};
