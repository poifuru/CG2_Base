#include "Model.h"
#include "ModelManager.h"
#include "TextureManager.h"

Model::Model(DxCommon* dxCommon, LightManager* lightManager)
	: BaseObject3d(dxCommon, lightManager){

}

Model::~Model() {

}

void Model::Initialize(const std::string& modelID) {
	// 親クラスの初期化
	BaseObject3d::Initialize();

	// === Modelクラス自身の初期化 === //
	// モデルの紐づけ
	auto modelData = ModelManager::GetInstance()->GetModelData(modelID);
	if(!modelData.expired()) {
		modelData_ = modelData.lock().get();
	}

	// 仮置き
	textureHandle_ = TextureManager::GetInstance()->GetTextureHandle("Dummy");
}

void Model::Update(CameraData* cameraData) {
	BaseObject3d::Update(cameraData);
}

void Model::Draw() {
	if(!modelData_) return;

	// RenderCommandの組み立て
	RenderCommand cmd{};


}
