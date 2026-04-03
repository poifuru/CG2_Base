#include "Game.h"
#include "CameraOrganizer.h"
#include "InputManager.h"
#include "DxCommon.h"
#include "BaseScene.h"
#include "SceneType.h"

Game::Game() {
	magosuya_ = MagosuyaEngine::GetInstance();
	magosuya_->Initialize();
	std::unique_ptr<BaseScene> scene = std::make_unique<TestScene>();
	sceneManager_ = SceneManager::GetInstance();
	sceneManager_->SetNextScene(std::move(scene));
}

Game::~Game() {

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