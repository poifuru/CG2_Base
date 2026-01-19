#include "Game.h"
#include "CameraOrganizer.h"
#include "InputManager.h"
#include "DxCommon.h"

Game::Game() {
	magosuya_ = MagosuyaEngine::GetInstance();
	magosuya_->Initialize();
	sceneManager_ = std::make_unique<SceneManager>(
		CameraOrganizer::GetInstance(), InputManager::GetInstance(), DxCommon::GetInstance()
	);
	sceneManager_->Initialize(SceneLabel::Title);
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