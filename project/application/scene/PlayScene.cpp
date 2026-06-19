#include "PlayScene.h"
#include "ShaderManager.h"
#include "RenderSystem.h"
#include "TextureManager.h"
#include "ModelFactory.h"

void PlayScene::Initialize() {
	if (!context_) return;

	// 1. シェーダーのコンパイルとキャッシュ登録
	vsID_ = context_->shaderManager->CompileAndCacheShader(L"Resources/shader/Triangle.VS.hlsl", L"vs_6_0");
	psID_ = context_->shaderManager->CompileAndCacheShader(L"Resources/shader/Triangle.PS.hlsl", L"ps_6_0");

	// 2. テクスチャをロード
	textureIndex_ = context_->textureManager->LoadTexture(
		"Resources/uvChecker.png",
		"MonsterTex",
		context_->device,
		context_->cmdList,
		*context_->heapManager
	);

	// 3. ファクトリ経由で三角形モデルを生成
	triangleModel_ = context_->modelFactory->CreateModel("Triangle", "MonsterTex");
	triangleModel_->SetShaders(vsID_, psID_);
	triangleModel_->SetTextureIndex(textureIndex_);
}

void PlayScene::Update(CameraData* cameraData) {
	if (triangleModel_) {
		triangleModel_->Update(cameraData);
	}
}

void PlayScene::Draw(RenderSystem* renderSystem) {
	if (triangleModel_) {
		triangleModel_->Draw(renderSystem);
	}
}