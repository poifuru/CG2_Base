#pragma once
#include "Scene.h"
#include <memory>
#include "Sprite.h"
#include "Model.h"
#include "player.h"
#include "Boss.h"
#include "DxCommon.h"
#include "../Enemy/EnemyManager.h"
#include "../Collider/CollisionManager.h"

class PlayScene : public Scene {
public:		//メンバ関数
	PlayScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon);
	~PlayScene () override;

	void Initialize () override;
	void Update () override;
	void Draw () override;

private:	//メンバ変数
	std::unique_ptr<Player>player_ = nullptr;
	std::unique_ptr<Boss> boss_ = nullptr;
	std::unique_ptr<Model> ground_ = nullptr;
	std::unique_ptr<Model> mountain_ = nullptr;
	std::unique_ptr<Model> stone_ = nullptr;
	std::unique_ptr<Model> skydome_ = nullptr;
	std::unique_ptr<EnemyManager>enemies_ = nullptr;
	std::unique_ptr<CollisionManager>collisionManager_ = nullptr;
};
