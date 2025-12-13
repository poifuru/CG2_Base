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
	std::unique_ptr<Sprite> gameover_ = nullptr;

	//ゲームシーンのカメラの位置
	Vector3 gameCameraPos_ = {};
	Vector3 gameCameraRotate_ = {};
	const Vector3 cameraGoalPos_ = { 0.0f, 0.0f, -50.0f };
	const Vector3 cameraGoalRotate_ = { -0.17f, 0.23f, 0.0f };

	float t_ = 0.0f;
};