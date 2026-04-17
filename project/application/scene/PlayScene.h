#pragma once
#include "BaseScene.h"
#include <memory>
#include "Model.h"
#include "LightManager.h"

class PlayScene : public BaseScene {
public:		//メンバ関数
	PlayScene ();
	~PlayScene () override;

	void Initialize (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) override;
	void Update () override;
	void Draw () override;
	void StopToResources() override;

private:
	// 敵を生成する関数
	void GenerateEnemies();

	bool CheckCollisionAABB(const AABB& a, const AABB& b) {
		return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
			(a.min.y <= b.max.y && a.max.y >= b.min.y) &&
			(a.min.z <= b.max.z && a.max.z >= b.min.z);
	}

private:	//メンバ変数
	std::unique_ptr<LightManager> lightManager_ = nullptr;
	std::unique_ptr<Model> skydome_ = nullptr;

	DxCommon* dxCommon_ = nullptr;
};
