#pragma once
#include <memory>
#include <string>
#include "Windows.h"
#include "DxCommon.h"
#include "SRVManager.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include "ModelManager.h"
#include "PSOManager.h"
#include "LineRenderer.h"
#include "CubeRenderer.h"
#include "Mesh.h"
#include "CameraOrganizer.h"
#include "SceneManager.h"
#include "AudioManager.h"
#include "PostEffectManager.h"

class MagosuyaEngine {
public:		//メンバ関数
	static MagosuyaEngine* GetInstance() {
		//初めて呼び出されたときに一回だけ初期化
		static MagosuyaEngine instance;
		return &instance;
	}
	~MagosuyaEngine();

	void Initialize();
	void BeginFrame();
	void EndFrame();

private:
	//コンストラクタを禁止
	MagosuyaEngine() = default;
	// コピーコンストラクタと代入演算子を禁止
	MagosuyaEngine(const MagosuyaEngine&) = delete;
	MagosuyaEngine& operator=(const MagosuyaEngine&) = delete;
	MagosuyaEngine(MagosuyaEngine&&) = delete;
	MagosuyaEngine& operator=(MagosuyaEngine&&) = delete;

private:
	WindowsAPI* winApi_ = nullptr;
	DxCommon* dxCommon_ = nullptr;
	SRVManager* srvManager_ = nullptr;
	ShaderManager* shaderManager_ = nullptr;
	RootSignatureManager* rootSigManager_ = nullptr;
	BlendModeManager* blendModeManager_ = nullptr;
	InputLayoutManager* inputLayoutManager_ = nullptr;
	PSOManager* psoManager_ = nullptr;
	ImGuiManager* imguiManager_ = nullptr;
	InputManager* inputManager_ = nullptr;
	TextureManager* texManager_ = nullptr;
	ModelManager* modelManager_ = nullptr;
	LineRenderer* lineRenderer_ = nullptr;
	CubeRenderer* cubeRenderer_ = nullptr;
	CameraOrganizer* cameraOrganizer_ = nullptr;
	SceneManager* sceneManager_ = nullptr;
	AudioManager* audioManager_ = nullptr;
	PostEffectManager* postEffect_ = nullptr;
};