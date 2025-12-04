#include "ModelRenderer.h"
#include <imgui.h>
#include "mathFunction.h"
#include "DxCommon.h"

ModelRenderer::ModelRenderer (DxCommon* dxCommon) {
	dxCommon_ = dxCommon;
	commandList_ = dxCommon->GetCommandList ();
ModelRenderer::ModelRenderer (MagosuyaEngine* magosuya) {
	modelCount_++;
	//その時のカウントをinstanceIDにコピー
	instanceID_ = modelCount_;
	magosuya_ = magosuya;
	rootSignature_ = magosuya_->GetDxCommon ()->GetRootSignature ();
	pipelineState_ = magosuya_->GetDxCommon ()->GetPipelineState ();
	for (int i = 0; i < 4; ++i) {
		color_[i] = 1.0f;
	}
}

ModelRenderer::~ModelRenderer () {
}

void ModelRenderer::Initialize () {
	//===リソースの初期化===//
	//行列データ
	matrixBuffer_ = dxCommon_->CreateBufferResource (sizeof (TransformationMatrix));
	matrixBuffer_->Map (0, nullptr, reinterpret_cast<void**>(&matrixData_));
	matrixData_->World = Math::MakeIdentity4x4 ();
	matrixData_->WVP = Math::MakeIdentity4x4 ();
	matrixData_->WorldInverseTranspose = Math::MakeIdentity4x4 ();

	//マテリアルデータ
	materialBuffer_ = dxCommon_->CreateBufferResource (sizeof (Material));
	materialBuffer_->Map (0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->enableLighting = true;
	materialData_->uvTranform = Math::MakeIdentity4x4 ();

	//PSO設定
	desc_.RootSignatureID = RootSignatureManager::GetInstance ()->GetOrCreateRootSignature (RootSigType::Standard3D);
	desc_.VS_ID = ShaderManager::GetInstance ()->CompileAndCasheShader (L"Resources/shader/Object3d.VS.hlsl", L"vs_6_0");
	desc_.PS_ID = ShaderManager::GetInstance ()->CompileAndCasheShader (L"Resources/shader/Object3d.PS.hlsl", L"ps_6_0");
	desc_.InputLayoutID = InputLayoutType::Standard3D;
	desc_.BlendMode = BlendModeType::Opaque;
}

void ModelRenderer::Update (Matrix4x4 world, Matrix4x4 vp, Transform uvTransform) {
	matrixData_->World = world;
	matrixData_->WVP = Math::Multiply(matrixData_->World, vp);
	matrixData_->WorldInverseTranspose = Math::Transpose (Math::Inverse (matrixData_->World));

	//uvTranform更新
	materialData_->uvTranform = Math::MakeAffineMatrix (uvTransform.scale, uvTransform.rotate, uvTransform.translate);
}

void ModelRenderer::Draw (D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) {
	// 共有データをロックして有効性をチェック
	std::shared_ptr<ModelData> data = modelData_.lock ();
	if (!data) {
		// モデルデータが解放済みなら描画をスキップ
		return;
	}
	RootSignatureManager::GetInstance ()->SetRootSignature (desc_.RootSignatureID);
	PSOManager::GetInstance ()->SetPSO (desc_);
	//どんな形状で描画するのか
	commandList_->IASetPrimitiveTopology (D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//頂点バッファをセットする
	commandList_->IASetVertexBuffers (0, 1, &data->vbView);
	//インデックスバッファをセットする
	commandList_->IASetIndexBuffer (&data->ibView);
	//定数バッファのルートパラメータを設定する	
	commandList_->SetGraphicsRootConstantBufferView (0, matrixBuffer_->GetGPUVirtualAddress ());
	commandList_->SetGraphicsRootConstantBufferView (1, materialBuffer_->GetGPUVirtualAddress ());
	//テクスチャのSRVを設定
	commandList_->SetGraphicsRootDescriptorTable (2, textureHandle);
	//実際に描画する(後々Index描画に変える)
	commandList_->DrawIndexedInstanced (static_cast<UINT>(data->indexCount), 1, 0, 0, 0);
}

void ModelRenderer::ImGui (Transform& transform, Transform& uvTransform, const std::string& windowName) {
	std::string num = std::to_string (instanceID_);
	std::string label = "##" + tag_ + num;
	ImGui::Text (("obj : " + windowName).c_str ());
	if (ImGui::ColorEdit4 (("Color" + label).c_str (), color_)) {
		// 色が変更されたらmaterialDataに反映
		materialData_->color.x = color_[0];
		materialData_->color.y = color_[1];
		materialData_->color.z = color_[2];
		materialData_->color.w = color_[3];
	}
	ImGui::DragFloat3 (("scale" + label).c_str (), &transform.scale.x, 0.01f);
	ImGui::DragFloat3 (("rotate" + label).c_str (), &transform.rotate.x, 0.01f);
	ImGui::DragFloat3 (("translate" + label).c_str (), &transform.translate.x, 0.01f);
	ImGui::DragFloat3 (("UVscale" + label).c_str (), &uvTransform.scale.x, 0.01f);
	ImGui::DragFloat3 (("UVrotate" + label).c_str (), &uvTransform.rotate.x, 0.01f);
	ImGui::DragFloat3 (("UVtranslate" + label).c_str (), &uvTransform.translate.x, 0.01f);
	//ライトの種類を選べるようにする
	int currentNum = 0;
	const char* lights[] = { "None", "lambert", "halfLambert" };
	if (ImGui::Combo (("ライティング" + label).c_str(), &currentNum, lights, IM_ARRAYSIZE (lights))) {
		if (currentNum == 0) {
			materialData_->enableLighting = Light::none;
			currentNum = 0;
		}
		else if (currentNum == 1) {
			materialData_->enableLighting = Light::lambert;
			currentNum = 1;
		}
		else if (currentNum == 2) {
			materialData_->enableLighting = Light::halfLambert;
			currentNum = 2;
		}
	}
	ImGui::Separator ();
}