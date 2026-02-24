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
	transform_ = { scale, rotate, position };
	uvTransform_ = { { 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f } };
	renderer_->Initialize ();
}

void Model::Update (CameraData* cameraData) {


	Matrix4x4 world = Math::MakeAffineMatrix (transform_.scale, transform_.rotate, transform_.translate);

	renderer_->Update (world, cameraData->vp, uvTransform_, cameraData->transform.translate);
}

void Model::Draw () {
	renderer_->Draw (texture_);
}

void Model::ImGui (const std::string& windowName) {
	renderer_->ImGui (transform_, uvTransform_, windowName);
}

void Model::SetModelData (const std::string& ID) {
	//マネージャーから受け取るモデルデータ
	std::weak_ptr<ModelData> modelData = ModelManager::GetInstance()->GetModelData (ID);
	renderer_->SetModelData (modelData);
	renderer_->SetImGuiID (ID); 
}

void Model::SetTexture (const std::string& ID) {
	texture_ = TextureManager::GetInstance()->GetTextureHandle (ID);
}

void Model::SetAnimation(const std::string& ID) {
	std::weak_ptr<Animation> animationData = ModelManager::GetInstance()->GetAnimationData(ID);
	renderer_->SetAnimation(animationData);
}