#include "ModelFactory.h"
#include "Model.h"
#include "ModelManager.h"
#include "TextureManager.h"

void ModelFactory::Initialize(DxCommon* dxCommon) {
	dxCommon_ = dxCommon;
}

std::unique_ptr<Model> ModelFactory::CreateModel(const std::string& modelName, const std::string& textureName) {
	// 1. 各マネージャーから安全にアセットのパーツを取得
	std::weak_ptr<ModelData> modelData = ModelManager::GetInstance()->GetModelData(modelName);
	auto tempModelData = modelData.lock().get();
	
	// 2. Modelインスタンスを生成
	auto model = std::make_unique<Model>(dxCommon_);

	// 3. パーツを流し込んで完全に初期化された状態にする
	model->Initialize(tempModelData);
	model->SetTexture(textureName);

	return model;
}
