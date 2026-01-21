#include "ModelRenderer.h"
#include <imgui.h>
#include "mathFunction.h"
#include "LightManager.h"
#include "SRVManager.h"

ModelRenderer::ModelRenderer (DxCommon* dxCommon, LightManager* lightManager) {
	dxCommon_ = dxCommon;
	commandList_ = dxCommon->GetCommandList ();
	lightManager_ = lightManager;
	modelCount_++;
	//その時のカウントをinstanceIDにコピー
	instanceID_ = modelCount_;
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
	materialData_->enableLighting = LightReflectionModel::HalfLambert;
	materialData_->uvTransform = Math::MakeIdentity4x4 ();
	materialData_->shininess = 50.0f;
	materialData_->isSpecular = false;

	cameraBuffer_ = dxCommon_->CreateBufferResource(sizeof(Vector3));
	cameraBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));
	cameraData_ = {};

	//PSO設定
	desc_.RootSignatureID = RootSignatureManager::GetInstance ()->GetOrCreateRootSignature (RootSigType::Standard3D);
	desc_.VS_ID = ShaderManager::GetInstance ()->CompileAndCasheShader (L"Resources/shader/Object3d.VS.hlsl", L"vs_6_0");
	desc_.PS_ID = ShaderManager::GetInstance ()->CompileAndCasheShader (L"Resources/shader/Object3d.PS.hlsl", L"ps_6_0");
	desc_.InputLayoutID = InputLayoutType::Standard3D;
	desc_.BlendMode = BlendModeType::Alpha;
}

void ModelRenderer::Update (Matrix4x4 world, Matrix4x4 vp, Transform uvTransform, Vector3 cameraWorld) {
	// 共有データをロックして有効性をチェック
	std::shared_ptr<ModelData> data = modelData_.lock ();
	if (!data) {
		// モデルデータが解放済みなら描画をスキップ
		return;
	}

	//RootのMatrixを適用する
	matrixData_->World = data->rootNode.localMatrix * world;
	matrixData_->WVP = matrixData_->World * vp;
	matrixData_->WorldInverseTranspose = Math::Transpose (Math::Inverse (matrixData_->World));

	//uvTranform更新
	materialData_->uvTransform = Math::MakeAffineMatrix (uvTransform.scale, uvTransform.rotate, uvTransform.translate);

	cameraData_ = &cameraWorld;
}

void ModelRenderer::Draw (D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) {
	// 共有データをロックして有効性をチェック
	std::shared_ptr<ModelData> data = modelData_.lock ();
	if(!data) {
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
	//カメラのPositionをセット
	commandList_->SetGraphicsRootConstantBufferView(2, cameraBuffer_->GetGPUVirtualAddress());
	//ライトの個数をセット
	commandList_->SetGraphicsRootConstantBufferView(3, lightManager_->GetLightCountBuffer().GetGPUVirtualAddress());
	//テクスチャのSRVを設定
	commandList_->SetGraphicsRootDescriptorTable (4, textureHandle);
	//directionalLightのSRVをセット
	commandList_->SetGraphicsRootDescriptorTable(
		5, SRVManager::GetInstance()->GetGPUDescriptorHandle(lightManager_->GetDirLightSrvHandle())
	);
	//pointLightのSRVをセット
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(
		6, SRVManager::GetInstance()->GetGPUDescriptorHandle(lightManager_->GetPointLightSrvHandle())
	);
	//spotLightのSRVをセット
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(
		7, SRVManager::GetInstance()->GetGPUDescriptorHandle(lightManager_->GetSpotLightSrvHandle())
	);
	//実際に描画する
	commandList_->DrawIndexedInstanced (static_cast<UINT>(data->indexCount), 1, 0, 0, 0);
}

void ModelRenderer::ImGui (Transform& transform, Transform& uvTransform, const std::string& windowName) {
#ifdef USEIMGUI
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
	ImGui::DragFloat(("shininess" + label).c_str(), &materialData_->shininess, 0.1f, 0.0f, 1000.0f);
	//ライトの種類を選べるようにする
	int currentNum = static_cast<int>(materialData_->enableLighting);
	const char* lights[] = { "None", "lambert", "halfLambert" };
	if(ImGui::Combo(("ライティング" + label).c_str(), &currentNum, lights, IM_ARRAYSIZE(lights))) {
		// 選ばれた番号をそのまま enum にキャストして戻せばOK！
		materialData_->enableLighting = static_cast<LightReflectionModel>(currentNum);
	}
	ImGui::Checkbox(("IsSpecular" + label).c_str(), &materialData_->isSpecular);
	ImGui::Separator ();
#endif
}