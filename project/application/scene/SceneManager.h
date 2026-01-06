#pragma once
#include "Scene.h"
#include <memory>
#include <string>
#include "TitleScene.h"
#include "PlayScene.h"
#include "ClearScene.h"
#include "GameoverScene.h"

class SceneManager {
public:		//メンバ関数
	SceneManager (CameraOrganizer* camera, InputManager* input, DxCommon* dxCommon);
	~SceneManager ();

	void Initialize (SceneLabel scene);
	void Update ();
	void Draw ();

private:	//メンバ変数
	//それぞれのシーンのポインタ
	std::unique_ptr<TitleScene> titleScene_;
	std::unique_ptr<PlayScene> playScene_;
	std::unique_ptr<ClearScene> clearScene_;
	std::unique_ptr<GameoverScene> gameoverScene_;

	//天球はずっと置いておきたい
	std::unique_ptr<Model> skydome_ = nullptr;
	float rotate_ = 0.0f;

	//平行光源のResourceを作成してデフォルト値を書き込む
	ComPtr<ID3D12Resource> dierctionalLightResource_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;
	//ライティング用の変数
	Vector4 colorLight;

	//現在のシーン
	SceneLabel scene_;
	//処理を共通化するためのポインタ
	Scene* currentScene_;

	CameraOrganizer* camera_ = nullptr;
};
