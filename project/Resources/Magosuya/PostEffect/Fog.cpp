#include "Fog.h"
#include "CameraOrganizer.h"
#include "SRVManager.h"
#include "MathFunction.h"
#include "imgui.h"

void Fog::Initialize(DxCommon* dxCommon) {
	BasePostEffect::Initialize(dxCommon);
	psoDesc_.PS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(
		L"Resources/shader/Fog.PS.hlsl", L"ps_6_0"
	);

	// リソースの初期化
	constantBuffer_ = dxCommon_->CreateBufferResource(sizeof(FogForGPU));
	constantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&cpuData_));

	cpuData_->color = { 0.0f, 0.782f, 1.0f, 0.9f };
	cpuData_->start = 50.0f;
	cpuData_->end = 80.0f;
	cpuData_->cameraNear = 0.1f;
	cpuData_->cameraFar = 1000.0f;
	cpuData_->heightStart = -20.0f;
	cpuData_->heightEnd = -50.0f;

	// 初期化時にスロットを2つ固定でアロケートする（確実に連続する）
	srvIndexSceneColor_ = SRVManager::GetInstance()->Allocate();
	srvIndexDepth_ = SRVManager::GetInstance()->Allocate();
}

void Fog::Draw(RenderTexture* renderTexture, CameraOrganizer* camera) {
	if(camera) {
		cpuData_->cameraNear = camera->GetNear();
		cpuData_->cameraFar = camera->GetFar();
	}
	else {
		cpuData_->cameraNear = 0.1f;
		cpuData_->cameraFar = 1000.0f;
	}

	auto cmdList = dxCommon_->GetCommandList();
	auto srvManager = SRVManager::GetInstance();
	auto depthResource = dxCommon_->GetDSV();

	// 確保したインデックスに対して、その場でSRVをそれぞれ生成する
	// シーンカラー (引数で渡ってきたレンダーテクスチャ)
	srvManager->CreateSRVforRenderTexture(
		srvIndexSceneColor_,
		renderTexture->GetResource(),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

	// デプスバッファ
	D3D12_SHADER_RESOURCE_VIEW_DESC depthSrvDesc{};
	depthSrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; // D24_UNORM_S8_UINT用
	depthSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	depthSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	depthSrvDesc.Texture2D.MipLevels = 1;

	dxCommon_->GetDevice()->CreateShaderResourceView(
		depthResource, 
		&depthSrvDesc, 
		srvManager->GetCPUDescriptorHandle(srvIndexDepth_)
	);

	// デプスバッファを読み取り用にバリア遷移
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = depthResource;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE; // あるいは DEPTH_READ
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	cmdList->ResourceBarrier(1, &barrier);

	// 定数バッファ更新
	if (camera) {
		cpuData_->cameraNear = camera->GetNear();
		cpuData_->cameraFar  = camera->GetFar();
		cpuData_->inverseVP  = Math::Inverse(camera->GetVPMatrix());
		cpuData_->cameraPosition = camera->GetCameraData().transform.translate;
	} else {
		cpuData_->cameraNear = 0.1f;
		cpuData_->cameraFar  = 1000.0f;
		cpuData_->inverseVP  = Math::MakeIdentity4x4();
		cpuData_->cameraPosition = { 0.0f, 0.0f, 0.0f };
	}

	RootSignatureManager::GetInstance()->SetRootSignature(psoDesc_.RootSignatureID);
	PSOManager::GetInstance()->SetPSO(psoDesc_);

	// プリミティブトポロジーを設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// CBVをセット
	cmdList->SetGraphicsRootConstantBufferView(0, constantBuffer_->GetGPUVirtualAddress());

	// SRVを設定 (t0)
	SRVManager::GetInstance()->SetGraphicsRootDescriptorTable(1, srvIndexSceneColor_);

	// 描画
	cmdList->DrawInstanced(3, 1, 0, 0);

	// 次のフレームのためにデプスバッファを元に戻す
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	cmdList->ResourceBarrier(1, &barrier);
}

void Fog::ImGui() {
	ImGui::ColorEdit4("color##Fog", &cpuData_->color.x);
	ImGui::DragFloat("start##Fog", &cpuData_->start, 0.1f, 0.0f, 1000.0f);
	ImGui::DragFloat("end##Fog", &cpuData_->end, 0.1f, 0.0f, 1000.0f);
	ImGui::DragFloat("heightStart##Fog", &cpuData_->heightStart, 0.1f, -100.0f, 100.0f);
	ImGui::DragFloat("heightEnd##Fog", &cpuData_->heightEnd, 0.1f, -100.0f, 100.0f);
	ImGui::Text("cameraNear : %.3f", cpuData_->cameraNear);
	ImGui::Text("cameraFar : %.3f", cpuData_->cameraFar);
}