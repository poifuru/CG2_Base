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
	uint32_t vsID,
	uint32_t psID,
	uint32_t modelIndex,
	uint32_t textureIndex
) {
	// 各マネージャーからアセットパーツを取得
	auto modelData = modelManager_->GetModelData(modelIndex);
	auto tempModelData = modelData.lock().get();

	// Modelインスタンスを生成（この時点では空）
	auto model = std::make_unique<Model>();

	// 親クラス用のバッファを生成・初期化
	// Transformバッファをモデルへ設定
	auto transformBuffer = std::make_unique<TransformMatrixResource>();
	transformBuffer->Initialize(device_->GetDevice());
	model->SetTransformBuffer(std::move(transformBuffer));

	// 新しいマテリアルを作成して初期化
	auto material = std::make_shared<Material>();
	material->Initialize(device_, heapManager_);
	material->SetTextureIndex(textureIndex);
	material->SetShader(vsID, psID);
	// モデルにマテリアルをセット
	model->SetMaterial(material);

	// アセットの設定
	model->Initialize(tempModelData);

	return model;
}
