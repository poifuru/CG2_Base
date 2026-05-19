#include "OldModel.h"
#include <imgui.h>
#include "function.h"
#include "MathFunction.h"
#include "ModelManager.h"
#include "TextureManager.h"

OldModel::OldModel(DxCommon* dxCommon, LightManager* lightManager) {
	renderer_ = std::make_unique<ModelRenderer> (dxCommon, lightManager);
}

OldModel::~OldModel() {
}

void OldModel::Initialize (Vector3 scale, Vector3 rotate, Vector3 position) {
	transform_ = { scale, rotate, position };
	uvTransform_ = { { 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f } };
	renderer_->Initialize ();
}

void OldModel::Update (CameraData* cameraData) {


	Matrix4x4 world = Math::MakeAffineMatrix (transform_.scale, transform_.rotate, transform_.translate);

	renderer_->Update (world, cameraData->vp, uvTransform_, cameraData->transform.translate);
}

void OldModel::Draw () {
	renderer_->Draw (texture_);
}

void OldModel::ImGui (const std::string& windowName) {
	renderer_->ImGui (transform_, uvTransform_, windowName);
}

void OldModel::SetModelData (const std::string& ID) {
	//マネージャーから受け取るモデルデータ
	std::weak_ptr<ModelData> modelData = ModelManager::GetInstance()->GetModelData (ID);
	renderer_->SetModelData (modelData);
	renderer_->SetImGuiID (ID); 
}

void OldModel::SetTexture (const std::string& ID) {
	texture_ = TextureManager::GetInstance()->GetTextureHandle (ID);
}

void OldModel::SetAnimation(const std::string& ID) {
	std::weak_ptr<Animation> animationData = ModelManager::GetInstance()->GetAnimationData(ID);
	renderer_->SetAnimation(animationData);
}

void OldModel::SkeletonInit() {
	renderer_->SkeletonInit();
}