#pragma once
#include "BaseScene.h"
#include <memory>
#include "Sprite.h"
#include "Model.h"
#include "MeshParticle.h"
#include "Mesh.h"
#include "SphereMesh.h"

class TitleScene : public BaseScene {
public:		//メンバ関数
	TitleScene ();
	~TitleScene () override;

	void Initialize (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) override;
	void Update () override;
	void Draw () override;

private:	//メンバ変数
	std::unique_ptr<SphereMesh> sphere_ = nullptr;

	//平行光源のResourceを作成してデフォルト値を書き込む
	ComPtr<ID3D12Resource> dierctionalLightResource_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;
	//ライティング用の変数
	float colorLight[4];
};