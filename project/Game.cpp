#include "Game.h"
#include "CameraOrganizer.h"
#include "InputManager.h"
#include "DxCommon.h"
#include "BaseScene.h"
#include "SceneType.h"

Game::Game() {
	magosuya_ = MagosuyaEngine::GetInstance();
	magosuya_->Initialize();
	BaseScene* scene = new TitleScene();
	sceneManager_ = SceneManager::GetInstance();
	sceneManager_->SetNextScene(scene);
}

Game::~Game() {

}

void Game::Initialize() {
	
}

void Game::Run() {
	/*メインループ！！！！！！！！！*/
	//ウィンドウの×ボタンが押されるまでループ
	while(true) {

		if(WindowsAPI::GetInstance()->ProcessMessage()) {
			break;
		}

		//フレーム開始
		magosuya_->BeginFrame();

		//***更新処理***//
		sceneManager_->Update();
		//*************//

		//***描画処理***//
		sceneManager_->Draw();
		//*************//

		//フレーム終了
		magosuya_->EndFrame();
	}
}