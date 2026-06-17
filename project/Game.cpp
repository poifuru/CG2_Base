#include "Game.h"
#include "CameraOrganizer.h"
#include "BaseScene.h"
#include "SceneType.h"

Game::Game() {
	engine_ = std::make_unique<Engine>();
	engine_->Initialize();
	/*std::unique_ptr<BaseScene> scene = std::make_unique<PlayScene>();
	sceneManager_ = SceneManager::GetInstance();
	sceneManager_->SetNextScene(std::move(scene));*/
}

Game::~Game() {

}

void Game::Run() {
	/*メインループ！！！！！！！！！*/
	//ウィンドウの×ボタンが押されるまでループ
	while(true) {

		if(engine_->ProcessMessage()) {
			break;
		}

		//フレーム開始
		engine_->BeginFrame();

		//***更新処理***//
		//sceneManager_->Update();
		//*************//

		//***描画処理***//
		//sceneManager_->Draw();
		//*************//

		//フレーム終了
		engine_->EndFrame();
	}
}