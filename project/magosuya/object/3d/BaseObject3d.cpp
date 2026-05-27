#include "BaseObject3d.h"
#include "MathFunction.h"
#include "imgui.h"

BaseObject3d::BaseObject3d(DxCommon* dxCommon) {
	instanceID_++;

	dxCommon_ = dxCommon;

	transformBuffer_ = std::make_unique<TransformMatrixResource>();
	materialBuffer_ = std::make_unique<MaterialResource>();
}

void BaseObject3d::Initialize() {
	// 共通バッファの生成
	transformBuffer_->Initialize(dxCommon_);
	materialBuffer_->Initialize(dxCommon_);

	// CPUデータの初期化
	// デフォルト値のセット
	transform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	uvTransform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	materialData_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_.enableLighting = TRUE;
	materialData_.roughness = 0.3f;
	materialData_.metallic = 0.5f;
	materialData_.environmentCoefficient = 0.3f;
}

void BaseObject3d::Update(CameraData* cameraData) {
	// === 行列の計算とGPUへの転送をここで一括処理 ===
	Matrix4x4 world = CalculateWorldMatrix();
	transformMatrixData_.World = world;
	transformMatrixData_.WVP = world * cameraData->vp;
	transformMatrixData_.WorldInverseTranspose = Math::Inverse(Math::Transpose(world));

	materialData_.uvTransform = Math::MakeAffineMatrix(uvTransform_.scale, uvTransform_.rotate, uvTransform_.translate);

	// GPUに送る
	transformBuffer_->Update(transformMatrixData_);
	materialBuffer_->Update(materialData_);
}

void BaseObject3d::Draw() {

}

void BaseObject3d::ImGui(const std::string& label) {
#ifdef USEIMGUI
	// ImGui用
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
			// チェックが切り替わったら、元の変数に書き戻す
			materialData_.enableLighting = bEnableLighting;
		}
		ImGui::TreePop();
	}
#endif // USEIMGUI
}

void BaseObject3d::SetRenderType(RenderType type) {
	switch(type) {
	case RenderType::Standard:

		psoDesc_.RootSignatureID = RootSignatureManager::GetInstance()->GetOrCreateRootSignature(RootSigType::Standard3D);
		psoDesc_.VS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/Object3d.VS.hlsl", L"vs_6_0");
		psoDesc_.PS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/Object3d.PS.hlsl", L"ps_6_0");
		psoDesc_.InputLayoutID = InputLayoutType::Standard3D;
		renderType_ = RenderType::Standard;

		break;

	case RenderType::Skining:

		psoDesc_.RootSignatureID = RootSignatureManager::GetInstance()->GetOrCreateRootSignature(RootSigType::SkinningStandard3D);
		psoDesc_.VS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/SkinningObject3d.VS.hlsl", L"vs_6_0");
		psoDesc_.PS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/SkinningObject3d.PS.hlsl", L"ps_6_0");
		psoDesc_.InputLayoutID = InputLayoutType::SkinningStandard3D;
		renderType_ = RenderType::Skining;

		break;

	case RenderType::Skybox:
		psoDesc_.RootSignatureID = RootSignatureManager::GetInstance()->GetOrCreateRootSignature(RootSigType::Skybox);
		psoDesc_.VS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/Skybox.VS.hlsl", L"vs_6_0");
		psoDesc_.PS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/Skybox.PS.hlsl", L"ps_6_0");
		psoDesc_.InputLayoutID = InputLayoutType::Skybox;
		psoDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		renderType_ = RenderType::Skybox;
		break;
	}
}

void BaseObject3d::SetBlendMode(BlendModeType type) {
	psoDesc_.BlendMode = type;

	if(type == BlendModeType::Opaque) {
		layer_ = 0;
	}
	else {
		layer_ = 1;
	}
}

void BaseObject3d::SetTexture(const std::string& filePath) {
	texInfo_.filePath = filePath;
	texInfo_.index = TextureManager::GetInstance()->LoadTexture(filePath);
	textureHandle_ = TextureManager::GetInstance()->GetTextureHandle(texInfo_.index);
}

Matrix4x4 BaseObject3d::CalculateWorldMatrix() {
	return Math::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
}
