#pragma once
#include "Scene.h"
#include <memory>
#include "Sprite.h"
#include "Model.h"
#include "mapchip.h"
#include "mapChipRenderer.h"
#include "Player.h"
#include "Boss.h"

class PlayScene : public Scene {
public:		//メンバ関数
	PlayScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon);
	~PlayScene () override;

	void Initialize () override;
	void Update () override;
	void Draw () override;

private:	//メンバ変数
	
};
