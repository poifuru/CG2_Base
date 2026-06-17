#include "BasePostEffect.h"
#include "DxCommon.h"

void BasePostEffect::Initialize(DxCommon* dxCommon) {
	dxCommon_ = dxCommon;

	// PSOの設定
	psoDesc_.VS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/Fullscreen.VS.hlsl", L"vs_6_0");
	psoDesc_.RootSignatureID = RootSignatureManager::GetInstance()->GetOrCreateRootSignature(RootSigType::PostProcess);
	psoDesc_.InputLayoutID = InputLayoutType::PostProcess;
	psoDesc_.BlendMode = BlendModeType::Opaque;

	// ラスタライザステート
	psoDesc_.CullMode = D3D12_CULL_MODE_NONE; // カリングなし
	psoDesc_.FillMode = D3D12_FILL_MODE_SOLID;

	// デプスステンシルステート (2D描画なのでデプス無効)
	psoDesc_.DepthEnable = FALSE;
	psoDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

	// その他の設定
	psoDesc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc_.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	psoDesc_.NumRenderTargets = 1;
	psoDesc_.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc_.SampleCount = 1;
}