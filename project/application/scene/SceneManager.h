#pragma once
#include "BaseScene.h"
#include <memory>
#include <string>

class SceneManager {
public:		//メンバ関数
	static SceneManager* GetInstance() {
		//初めて呼び出されたときに一回だけ初期化
		static SceneManager instance;
		return &instance;
	}
	~SceneManager ();

	void Initialize(CameraOrganizer* camera, InputManager* input, DxCommon* dxCommon);
	void Update ();
	void Draw ();
	void DrawUI () { if(scene_) scene_->DrawUI(); }
	
	//次シーンの予約
	void SetNextScene(std::unique_ptr<BaseScene> nextScene) { nextScene_ = std::move(nextScene); }

private:
	//コンストラクタを禁止
	SceneManager() = default;
	// コピーコンストラクタと代入演算子を禁止
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;
	SceneManager(SceneManager&&) = delete;
	SceneManager& operator=(SceneManager&&) = delete;

private:	//メンバ変数
	//実行中のシーン
	std::unique_ptr<BaseScene> scene_ = nullptr;
	//次のシーン
	std::unique_ptr<BaseScene> nextScene_ = nullptr;

	CameraOrganizer* camera_ = nullptr;
	InputManager* input_ = nullptr;
	DxCommon* dxCommon_ = nullptr;
};
