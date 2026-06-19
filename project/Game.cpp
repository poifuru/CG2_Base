#include "Game.h"
#include "Engine.h"
#include "PlayScene.h"
#include "MathFunction.h"

#pragma comment(lib, "DirectXTex.lib")

Game::Game() {
	engine_ = std::make_unique<Engine>();
	engine_->Initialize();

	// Engineのポインタを取得（ゲッターを使うためキャスト）
	Engine* rawEngine = static_cast<Engine*>(engine_.get());

	// ロード用コマンドリストをリセットしてロード開始
	rawEngine->ResetCommandList();

	// SceneManagerの生成と初期化
	sceneManager_ = std::make_unique<SceneManager>();
	sceneManager_->Initialize(
		rawEngine->GetDevice(),
		rawEngine->GetGraphicsDevice(),
		rawEngine->GetCommandList(),
		rawEngine->GetDescriptorHeapManager(),
		&rawEngine->GetShaderManager(),
		rawEngine->GetInputManager()
	);

	// 初期シーンにPlayScene（三角形を描画するデモシーン）を設定
	sceneManager_->ChangeScene<PlayScene>();

	// コマンドリストを実行し、GPUのアップロード完了を待つ
	rawEngine->ExecuteCommandList();
}

Game::~Game() {
}

void Game::Run() {
	// ダミーのカメラデータを用意
	CameraData dummyCamera{};
	dummyCamera.vp = Math::MakeIdentity4x4();

	//ウィンドウの×ボタンが押されるまでループ
	while(true) {

		if(engine_->ProcessMessage()) {
			break;
		}

		//フレーム開始
		engine_->BeginFrame();

		// シーンの更新（マテリアルデータの転送もPlayScene内部で行われます）
		sceneManager_->Update(&dummyCamera);

		// シーンの描画コマンドの積み込み
		sceneManager_->Draw(engine_->GetRenderSystem());

		//フレーム終了
		engine_->EndFrame();
	}
}