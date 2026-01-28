#pragma once
#include "BaseScene.h"
#include <memory>
#include "Sprite.h"
#include "Model.h"
#include "mapchip.h"
#include "mapChipRenderer.h"
#include "Player.h"
#include "Boss.h"
#include "LightManager.h"
#include "Player.h"

class PlayScene : public BaseScene {
public:		//メンバ関数
	PlayScene ();
	~PlayScene () override;

	void Initialize (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) override;
	void Update () override;
	void Draw () override;

private:	//メンバ変数
	std::unique_ptr<LightManager> lightManager_ = nullptr;
	std::unique_ptr<MapChip> mapchip_ = nullptr;
	std::unique_ptr<Player> player_ = nullptr;
};
