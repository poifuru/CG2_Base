#include "MagosuyaEngine.h"

MagosuyaEngine::~MagosuyaEngine () {
	dxCommon_->Finalize ();
}

void MagosuyaEngine::Initialize () {
	winApi_ = WindowsAPI::GetInstance ();
	winApi_->Initialize (InputManager::GetInstance ());
	dxCommon_ = DxCommon::GetInstance ();
	dxCommon_->Initialize ();
	srvManager_ = SRVManager::GetInstance();
	srvManager_->Initialize(dxCommon_);
	imguiManager_ = ImGuiManager::GetInstance ();
	imguiManager_->Initialize ();
	inputManager_ = InputManager::GetInstance ();
	inputManager_->Initialize (winApi_->GetHwnd ());
	texManager_ = TextureManager::GetInstance ();
	texManager_->Initialize (dxCommon_);
	modelManager_ = ModelManager::GetInstance ();
	modelManager_->Initialize (dxCommon_, texManager_);
	psoManager_ = PSOManager::GetInstance ();
	psoManager_->Initialize (dxCommon_);
	lineRenderer_ = LineRenderer::GetInstance ();
	lineRenderer_->Initialize (dxCommon_);
	cubeRenderer_ = CubeRenderer::GetInstance ();
	cubeRenderer_->Initialize (dxCommon_);
	cameraOrganizer_ = CameraOrganizer::GetInstance ();
	cameraOrganizer_->Initialize (inputManager_);
}

void MagosuyaEngine::BeginFrame () {
	imguiManager_->BeginFrame ();
	dxCommon_->BeginFrame ();
	srvManager_->PreDraw();

	//ゲームパッドの更新
	inputManager_->GetGamePad ()->Update ();
}
void MagosuyaEngine::EndFrame () {
	Mesh::AllDrawing ();
	imguiManager_->Draw ();
	inputManager_->EndFrame ();
	dxCommon_->EndFrame ();
	texManager_->ClearIntermediateResource ();
}