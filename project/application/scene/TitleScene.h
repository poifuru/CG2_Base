#pragma once
#include "Scene.h"
#include <memory>
#include "Sprite.h"
#include "Model.h"
#include "MeshParticle.h"

class TitleScene : public Scene {
public:		//メンバ関数
	TitleScene (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon);
	~TitleScene () override;

	void Initialize () override;
	void Update () override;
	void Draw () override;

private:	//メンバ変数
	std::unique_ptr<Model> ground_ = nullptr;
	std::unique_ptr<Model> mountain_ = nullptr;
	std::unique_ptr<Model> stone_ = nullptr;
	std::unique_ptr<Model> skydome_ = nullptr;

	//火山噴火
	std::unique_ptr<MeshParticle> particle_ = nullptr;
	const float particleTimer_ = 3.0f;
	float particleTimeCount_ = 0.0f;

	//タイトルモデル
	std::unique_ptr<Model> moji_ = nullptr;
	std::unique_ptr<Model> zako_ = nullptr;
};
