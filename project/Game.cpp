#include "PCH.h"
#include "Game.h"
#include "Engine.h"
#include "PlayScene.h"
#include "CameraOrganizer.h"
#include "ImGuiManager.h"

#pragma comment(lib, "DirectXTex.lib")

Game::Game() {
	engine_ = std::make_unique<Engine>();
	engine_->Initialize();

	// ロード用コマンドリストをリセットしてロード開始
	engine_->ResetCommandList();

	// カメラの初期化
	CameraOrganizer::GetInstance()->Initialize();

	// SceneManagerの生成と初期化
	sceneManager_ = std::make_unique<SceneManager>();
	sceneManager_->Initialize(
		engine_->GetDevice(),
		engine_->GetGraphicsDevice(),
		engine_->GetCommandList(),
		engine_->GetDescriptorHeapManager(),
		&engine_->GetShaderManager()
	);

	// 初期シーンにPlayScene（三角形を描画するデモシーン）を設定
	sceneManager_->ChangeScene<PlayScene>();

	// コマンドリストを実行し、GPUのアップロード完了を待つ
	engine_->ExecuteCommandList();

	// ImGuiの初期化
	ImGuiManager::GetInstance()->Initialize(engine_.get());
}

Game::~Game() {
	ImGuiManager::GetInstance()->Finalize();
}

void Game::Run() {
	//ウィンドウの×ボタンが押されるまでループ
	while(true) {

		if(engine_->ProcessMessage()) {
			break;
		}

		// ImGui 新しいフレーム開始
		ImGuiManager::GetInstance()->BeginFrame();

		//フレーム開始
		engine_->BeginFrame();

		sceneManager_->Update(&CameraOrganizer::GetInstance()->GetCameraData());

		// シーンの描画コマンドの積み込み
		sceneManager_->Draw(engine_->GetRenderSystem());

		// シーン描画の実行とSwapChainへの切り替え
		engine_->PreImGui();

		// ImGuiの描画コマンド積み込み
		ImGuiManager::GetInstance()->Draw();

		//フレーム終了
		engine_->EndFrame();
	}
}