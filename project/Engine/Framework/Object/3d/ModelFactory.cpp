#include "ModelFactory.h"
#include "Model.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "GraphicsDevice.h"
#include "DescriptorHeapManager.h"
#include "StructuredBuffer.h"
#include "ShaderManager.h"

void ModelFactory::Initialize(
	GraphicsDevice* device,
	DescriptorHeapManager* heapManager,
	ModelManager* modelManager,
	TextureManager* textureManager,
	ShaderManager* shaderManager
) {
	device_ = device;
	heapManager_ = heapManager;
	modelManager_ = modelManager;
	textureManager_ = textureManager;
	shaderManager_ = shaderManager;
}

std::unique_ptr<Model> ModelFactory::CreateModel(
	uint32_t modelIndex,
	uint32_t textureIndex
) {
	// 各マネージャーからアセットパーツを取得
	auto modelData = modelManager_->GetModelData(modelIndex);
	auto tempModelData = modelData.lock().get();

	// デフォルトのシェーダーをコンパイル＆キャッシュ登録
	uint32_t vsID = shaderManager_->CompileAndCacheShader(L"Resources/shader/Object3d.VS.hlsl", L"vs_6_0");
	uint32_t psID = shaderManager_->CompileAndCacheShader(L"Resources/shader/Object3d.PS.hlsl", L"ps_6_0");

	// Modelインスタンスを生成（この時点では空）
	auto model = std::make_unique<Model>();

	// 親クラス用のバッファを生成・初期化
	// (ファクトリが持っているデバイスとヒープを使って初期化)
	auto transformBuffer = std::make_unique<TransformMatrixResource>();
	transformBuffer->Initialize(device_->GetDevice());

	auto materialBuffer = std::make_unique<StructuredBuffer<MaterialData>>();
	materialBuffer->Initialize(device_->GetDevice(), *heapManager_, 1);

	// 初期化済みのバッファをムーブで流し込む（Modelにはデバイスを持たせない）
	model->SetCommonBuffers(std::move(transformBuffer), std::move(materialBuffer));

	// アセットの設定
	model->Initialize(tempModelData);
	model->SetTextureIndex(textureIndex);
	model->SetShaders(vsID, psID);

	return model;
}
