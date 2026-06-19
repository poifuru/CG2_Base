#include "BaseObject3d.h"
#include "MathFunction.h"
#include "imgui.h"

BaseObject3d::BaseObject3d() {
	instanceID_++;
	transformBuffer_ = std::make_unique<TransformMatrixResource>();
	materialBuffer_ = std::make_unique<StructuredBuffer<MaterialData>>();
}

void BaseObject3d::Initialize() {
	transform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	uvTransform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	materialData_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_.enableLighting = TRUE;
	materialData_.roughness = 0.3f;
	materialData_.metallic = 0.5f;
	materialData_.environmentCoefficient = 0.3f;
}

void BaseObject3d::Update(CameraData* cameraData) {
	Matrix4x4 world = CalculateWorldMatrix();
	transformMatrixData_.World = world;
	transformMatrixData_.WVP = world * cameraData->vp;
	transformMatrixData_.WorldInverseTranspose = Math::Inverse(Math::Transpose(world));

	materialData_.uvTransform = Math::MakeAffineMatrix(uvTransform_.scale, uvTransform_.rotate, uvTransform_.translate);

	if (transformBuffer_) transformBuffer_->Update(transformMatrixData_);
	
	// 構造化バッファへのデータ転送
	if (materialBuffer_) {
		std::vector<MaterialData> list = { materialData_ };
		materialBuffer_->Update(list);
	}
}

void BaseObject3d::ImGui(const std::string& label) {
#ifdef USEIMGUI
	bool bEnableLighting = materialData_.enableLighting;

	if(ImGui::TreeNode("Transform")) {
		ImGui::DragFloat3(("Scale" + label).c_str(), &transform_.scale.x, 0.01f);
		ImGui::DragFloat3(("Rotate" + label).c_str(), &transform_.rotate.x, 0.01f);
		ImGui::DragFloat3(("Transform" + label).c_str(), &transform_.translate.x, 0.01f);
		ImGui::TreePop();
	}
	if(ImGui::TreeNode("Material")) {
		ImGui::ColorEdit4(("color" + label).c_str(), &materialData_.color.x);
		ImGui::DragFloat3(("uvScale" + label).c_str(), &uvTransform_.scale.x, 0.01f);
		ImGui::DragFloat3(("uvRotate" + label).c_str(), &uvTransform_.rotate.x, 0.01f);
		ImGui::DragFloat3(("uvTransform" + label).c_str(), &uvTransform_.translate.x, 0.01f);
		ImGui::SliderFloat(("Roughness" + label).c_str(), &materialData_.roughness, 0.0f, 1.0f);
		ImGui::SliderFloat(("Metallic" + label).c_str(), &materialData_.metallic, 0.0f, 1.0f);
		ImGui::SliderFloat(("EnvironmentCoefficient" + label).c_str(), &materialData_.environmentCoefficient, 0.0f, 1.0f);
		if(ImGui::Checkbox(("EnableLighting" + label).c_str(), &bEnableLighting)) {
			materialData_.enableLighting = bEnableLighting;
		}
		ImGui::TreePop();
	}
#endif // USEIMGUI
}

Matrix4x4 BaseObject3d::CalculateWorldMatrix() {
	return Math::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
}
