//#include "SceneManager.h"
//#include "CameraOrganizer.h"
//#include "ModelManager.h"
//
//SceneManager::~SceneManager () {
//	scene_->StopToResources();
//}
//
//void SceneManager::Initialize(CameraOrganizer* camera, InputManager* input, DxCommon* dxCommon) {
//	camera_ = camera;
//	input_ = input;
//	dxCommon_ = dxCommon;
//}
//
//void SceneManager::Update () {
//	//次のシーンの予約があるなら
//	if(nextScene_) {
//		//旧シーンを終了
//		if(scene_) {
//			scene_->StopToResources();
//		}
//
//		//シーン切り替え
//		scene_ = std::move(nextScene_);
//		//次シーンの初期化
//		scene_->Initialize(camera_, input_, dxCommon_);
//	}
//
//	if(scene_) {
//		scene_->Update();
//	}
//}
//
//void SceneManager::Draw () {
//	scene_->Draw ();
//}