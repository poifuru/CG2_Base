#include "ModelFactory.h"
#include "Model.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "GraphicsDevice.h"
#include "DescriptorHeapManager.h"
#include "StructuredBuffer.h"

ModelFactory::ModelFactory(
	GraphicsDevice* device,
	DescriptorHeapManager* heapManager,
	ModelManager* modelManager,
	TextureManager* textureManager
) : device_(device), heapManager_(heapManager), modelManager_(modelManager), textureManager_(textureManager) {
}

void ModelFactory::Initialize(
	GraphicsDevice* device,
	DescriptorHeapManager* heapManager,
	ModelManager* modelManager,
	TextureManager* textureManager
) {
	device_ = device;
	heapManager_ = heapManager;
	modelManager_ = modelManager;
	textureManager_ = textureManager;
}

std::unique_ptr<Model> ModelFactory::CreateModel(const std::string& modelName, const std::string& textureName) {
	// 1. 各マネージャーからアセットパーツとテクスチャインデックスを取得
	auto modelData = modelManager_->GetModelData(modelName);
	auto tempModelData = modelData.lock().get();

	uint32_t textureIndex = textureManager_->GetTextureIndex(textureName);

	// 2. Modelインスタンスを生成（この時点では空）
	auto model = std::make_unique<Model>();

	// 3. 親クラス用のバッファを生成・初期化
	// (ファクトリが持っているデバイスとヒープを使って初期化)
	auto transformBuffer = std::make_unique<TransformMatrixResource>();
	transformBuffer->Initialize(device_->GetDevice());

	auto materialBuffer = std::make_unique<StructuredBuffer<MaterialData>>();
	materialBuffer->Initialize(device_->GetDevice(), *heapManager_, 1);

	// 4. 初期化済みのバッファをムーブで流し込む（Modelにはデバイスを持たせない）
	model->SetCommonBuffers(std::move(transformBuffer), std::move(materialBuffer));

	// 5. アセットの設定
	model->Initialize(tempModelData);
	model->SetTextureIndex(textureIndex);

	return model;
}
