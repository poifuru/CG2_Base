#include "MagosuyaEngine.h"
#include "LogManager.h"
#include "PostEffect.h"
#include "Primitive.h"

MagosuyaEngine::~MagosuyaEngine () {
	audioManager_->Finalize();
	dxCommon_->Finalize ();
}

void MagosuyaEngine::Initialize () {
	LogManager::GetInstance()->Initialize();

	winApi_ = WindowsAPI::GetInstance ();
	winApi_->Initialize (InputManager::GetInstance ());

	dxCommon_ = DxCommon::GetInstance ();
	dxCommon_->Initialize ();

	srvManager_ = SRVManager::GetInstance();
	srvManager_->Initialize(dxCommon_);

	shaderManager_ = ShaderManager::GetInstance();
	shaderManager_->Initialize(dxCommon_);

	rootSigManager_ = RootSignatureManager::GetInstance();
	rootSigManager_->Initialize(dxCommon_);

	blendModeManager_ = BlendModeManager::GetInstance();
	blendModeManager_->Initialize();

	inputLayoutManager_ = InputLayoutManager::GetInstance();
	inputLayoutManager_->Initialize();

	psoManager_ = PSOManager::GetInstance();
	psoManager_->Initialize(dxCommon_);

	imguiManager_ = ImGuiManager::GetInstance ();
	imguiManager_->Initialize ();

	inputManager_ = InputManager::GetInstance ();
	inputManager_->Initialize (winApi_->GetHwnd ());

	texManager_ = TextureManager::GetInstance ();
	texManager_->Initialize (dxCommon_);

	modelManager_ = ModelManager::GetInstance ();
	modelManager_->Initialize (dxCommon_, texManager_);

	lineRenderer_ = LineRenderer::GetInstance ();
	lineRenderer_->Initialize (dxCommon_);

	cubeRenderer_ = CubeRenderer::GetInstance ();
	cubeRenderer_->Initialize (dxCommon_);

	cameraOrganizer_ = CameraOrganizer::GetInstance ();
	cameraOrganizer_->Initialize (inputManager_, dxCommon_);

	sceneManager_ = SceneManager::GetInstance();
	sceneManager_->Initialize(cameraOrganizer_, inputManager_, dxCommon_);

	audioManager_ = AudioManager::GetInstance();
	audioManager_->Initialize();

	// SRVManagerが初期化された後でオフスクリーン用のRenderTextureを初期化
	dxCommon_->InitializeRenderTexture(srvManager_);

	postEffect_ = PostEffect::GetInstance();
	postEffect_->Initialize(dxCommon_);

	modelFactory_ = ModelFactory::GetInstance();
	modelFactory_->Initialize(dxCommon_);

	renderSystem_ = RenderSystem::GetInstance();
	renderSystem_->Initialize(dxCommon_);
}

void MagosuyaEngine::BeginFrame () {
	dxCommon_->BeginFrame ();
	imguiManager_->BeginFrame ();
	srvManager_->PreDraw();

	//ゲームパッドの更新
	inputManager_->GetGamePad ()->Update ();
	audioManager_->Update();
}
void MagosuyaEngine::EndFrame () {
	Primitive::AllDrawing ();
	inputManager_->EndFrame ();

	// 描画を実行！
	renderSystem_->ExecuteCommands();
	// 描画が終わったらコマンドを空にする
	renderSystem_->ClearCommands();

	// ImGuiの描画の前に、描画先をRenderTextureからSwapchainへ切り替える
	dxCommon_->PreDrawImGui();

	// RenderTextureの内容をSwapchainにコピー
	postEffect_->Draw(dxCommon_->GetRenderTexture());

	imguiManager_->Draw ();
	dxCommon_->EndFrame ();
	texManager_->ClearIntermediateResource ();
}