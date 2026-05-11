#include "Model.h"
#include <imgui.h>
#include "function.h"
#include "MathFunction.h"
#include "ModelManager.h"
#include "TextureManager.h"

Model::Model (DxCommon* dxCommon, LightManager* lightManager) {
	renderer_ = std::make_unique<ModelRenderer> (dxCommon, lightManager);
}

Model::~Model () {
}

void Model::Initialize (Vector3 scale, Vector3 rotate, Vector3 position) {
	Transform_ = { scale, rotate, position };
	uvTransform_ = { { 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f } };
	renderer_->Initialize ();
}

void Model::Update (CameraData* cameraData) {
	Matrix4x4 world = Math::MakeAffineMatrix (Transform_.scale, Transform_.rotate, Transform_.translate);

	renderer_->Update (world, cameraData->vp, uvTransform_, cameraData->Transform.translate);
}

void Model::Draw () {
	renderer_->Draw (texture_);
}

void Model::ImGui (const std::string& windowName) {
	renderer_->ImGui (Transform_, uvTransform_, windowName);
}

void Model::SetModelData (const std::string& ID) {
	modelData_ = ModelManager::GetInstance()->GetModelData (ID);
	renderer_->SetModelData (modelData_);
	renderer_->SetImGuiID (ID); 
}

void Model::SetTexture (const std::string& ID) {
	texture_ = TextureManager::GetInstance()->GetTextureHandle (ID);
}