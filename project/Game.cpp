#include "PCH.h"
#include "Game.h"
#include "Engine.h"
#include "Renderer.h"
#include "PlayScene.h"
#include "CameraOrganizer.h"
#include "ImGuiManager.h"
#include "RenderTexture.h"

Game::Game() {
	engine_ = std::make_unique<MyEngine::LowLevel::Engine>();
	engine_->Initialize();

	renderer_ = std::make_unique<MyEngine::Rendering::Renderer>();
	renderer_->Initialize(
		engine_->GetDevice(),
		engine_->GetDxcUtils(),
		engine_->GetDxcCompiler(),
		engine_->GetIncludeHandler(),
		engine_->GetDescriptorHeapManager()
	);

	// ロード用コマンドリストをリセットしてロード開始
	engine_->ResetCommandList();

	// カメラの初期化
	CameraOrganizer::GetInstance()->Initialize();

	// SceneManagerの生成と初期化
	sceneManager_ = std::make_unique<SceneManager>();
	sceneManager_->Initialize(
		engine_->GetGraphicsDevice(),
		engine_->GetCommandList(),
		engine_->GetDescriptorHeapManager(),
		renderer_->GetShaderManager()
	);

	// 初期シーンの設定
	sceneManager_->ChangeScene<PlayScene>();
	sceneManager_->SetRenderSystem(renderer_->GetRenderSystem());

	// コマンドリストを実行し、GPUのアップロード完了を待つ
	engine_->ExecuteCommandList();

	// ImGuiの初期化
	ImGuiManager::GetInstance()->Initialize(
		engine_->GetDevice(),
		engine_->GetCommandQueue(),
		engine_->GetDescriptorHeapManager()
	);
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
		ImGuiManager::GetInstance()->BeginFrame(
			engine_->GetDescriptorHeapManager(),
			renderer_->GetRenderTexture()
		);

		//フレーム開始
		engine_->BeginFrame(
			renderer_->GetRenderTexture()->GetResource(),
			renderer_->GetRenderTexture()->GetDescriptorHandle()
		);

		// シーンの更新
		sceneManager_->Update(&CameraOrganizer::GetInstance()->GetCameraData());
		// シーンの描画コマンドの積み込み
		sceneManager_->Draw(renderer_->GetRenderSystem());

		// rendererで実際に描画
		renderer_->RenderScene(engine_->GetCommandList());

		// SwapChainの切り替え(USEIMGUI時)
		engine_->BeginSwapChainRender();

		// ImGuiの描画コマンド積み込み
		ImGuiManager::GetInstance()->Draw(engine_->GetCommandList());

		//フレーム終了
		engine_->EndFrame();
	}
}