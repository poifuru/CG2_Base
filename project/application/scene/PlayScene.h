#pragma once
#include "Scene.h"
#include <memory>
#include "Sprite.h"
#include "Model.h"
#include "mapchip.h"
#include "mapChipRenderer.h"

class PlayScene : public Scene {
public:		//メンバ関数
	PlayScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon);
	~PlayScene () override;

	void Initialize () override;
	void Update () override;
	void Draw () override;

private:	//メンバ変数
	//マップチップ
	std::unique_ptr<Model> map_ = nullptr;

	//平行光源のResourceを作成してデフォルト値を書き込む
	ComPtr<ID3D12Resource> dierctionalLightResource_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;
	//ライティング用の変数
	Vector4 colorLight;
};
