#pragma once
#include "BaseScene.h"
#include <memory>
#include "Model.h"
#include "LightManager.h"
#include "Player.h"
#include "EnemyManager.h"
#include "Skybox.h"

class PlayScene : public BaseScene {
public:		//メンバ関数
	PlayScene ();
	~PlayScene () override;

	void Initialize (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) override;
	void Update () override;
	void Draw () override;
	void StopToResources() override;

private:	//メンバ変数
	std::unique_ptr<LightManager> lightManager_ = nullptr;

	//std::unique_ptr<Player> player_ = nullptr;
	//std::unique_ptr<EnemyManager> enemyManager_ = nullptr;

	std::unique_ptr<Skybox> skybox_ = nullptr;

	DxCommon* dxCommon_ = nullptr;
};
