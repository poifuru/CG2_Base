#include "BaseObject3d.h"
#include "MathFunction.h"
#include "imgui.h"

BaseObject3d::BaseObject3d() {
	instanceID_++;
	transformBuffer_ = std::make_unique<TransformMatrixResource>();
}

void BaseObject3d::Initialize() {
	transform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
}

void BaseObject3d::Update(CameraData* cameraData) {
	Matrix4x4 world = CalculateWorldMatrix();
	transformMatrixData_.World = world;
	transformMatrixData_.WVP = world * cameraData->vp;
	transformMatrixData_.WorldInverseTranspose = Math::Inverse(Math::Transpose(world));

	if (transformBuffer_) transformBuffer_->Update(transformMatrixData_);
	
	// 構造化バッファへのデータ転送
	if (material_) {
		material_->Update();
	}
}

void BaseObject3d::ImGui(const std::string& label) {
#ifdef USEIMGUI
	if(!material_) return;

	// コピーを取り出してImGuiで編集
	MaterialData materialData = material_->GetMaterialData();
	EulerTransform uvTransform = material_->GetUvTransform();

	if(ImGui::TreeNode("Transform")) {
		ImGui::DragFloat3(("Scale" + label).c_str(), &transform_.scale.x, 0.01f);
		ImGui::DragFloat3(("Rotate" + label).c_str(), &transform_.rotate.x, 0.01f);
		ImGui::DragFloat3(("Transform" + label).c_str(), &transform_.translate.x, 0.01f);
		ImGui::TreePop();
	}
	if(ImGui::TreeNode("Material")) {
		// 編集されたらセッター経由でマテリアルに反映して isDirty_ を立てる
		if (ImGui::ColorEdit4(("color" + label).c_str(), &materialData.color.x)) {
			material_->SetColor(materialData.color);
		}
		if (ImGui::DragFloat3(("uvScale" + label).c_str(), &uvTransform.scale.x, 0.01f)) {
			material_->SetUvScale(uvTransform.scale);
		}
		if (ImGui::DragFloat3(("uvRotate" + label).c_str(), &uvTransform.rotate.x, 0.01f)) {
			material_->SetUvRotate(uvTransform.rotate);
		}
		if (ImGui::DragFloat3(("uvTransform" + label).c_str(), &uvTransform.translate.x, 0.01f)) {
			material_->SetUvTranslate(uvTransform.translate);
		}
		if (ImGui::SliderFloat(("Roughness" + label).c_str(), &materialData.roughness, 0.0f, 1.0f)) {
			material_->SetRoughness(materialData.roughness);
		}
		if (ImGui::SliderFloat(("Metallic" + label).c_str(), &materialData.metallic, 0.0f, 1.0f)) {
			material_->SetMetallic(materialData.metallic);
		}
		if (ImGui::SliderFloat(("EnvironmentCoefficient" + label).c_str(), &materialData.environmentCoefficient, 0.0f, 1.0f)) {
			material_->SetEnvironmentCoefficient(materialData.environmentCoefficient);
		}
		
		bool bEnableLighting = materialData.enableLighting;
		if(ImGui::Checkbox(("EnableLighting" + label).c_str(), &bEnableLighting)) {
			material_->SetEnableLighting(bEnableLighting);
		}
		ImGui::TreePop();
	}
#endif // USEIMGUI
}

Matrix4x4 BaseObject3d::CalculateWorldMatrix() {
	return Math::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
}
