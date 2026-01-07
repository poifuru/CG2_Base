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
	std::unique_ptr<Model> gameover_ = nullptr;

	//平行光源のResourceを作成してデフォルト値を書き込む
	ComPtr<ID3D12Resource> dierctionalLightResource_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;
	//ライティング用の変数
	Vector4 colorLight;
};