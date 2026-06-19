#include "RootSignatureManager.h"
#include <cassert>
#include <dxcapi.h>

void RootSignatureManager::Initialize (ID3D12Device* device) {
	assert(device != nullptr);
	HRESULT hr = S_OK;

	// === バインドレス用共通ルートパラメータの設定（4スロット固定） ===
	D3D12_ROOT_PARAMETER rootParameters[4] = {};

	// Slot0 : フレーム共通定数バッファ (b0, space0) -> Vertex, Pixel両方から見えるようにする
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[0].Descriptor.ShaderRegister = 0;
	rootParameters[0].Descriptor.RegisterSpace = 0;

	// Slot1 : オブジェクトインデックス送信 (b1, space0) -> 32ビット定数として2個の数値を送る
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[1].Constants.ShaderRegister = 1;
	rootParameters[1].Constants.RegisterSpace = 0;
	rootParameters[1].Constants.Num32BitValues = 2; // materialIndex と textureIndex の2つ

	// Slot2 : バインドレスヒープ全体を指すディスクリプタテーブル (t0, space1)
	// レンジの設定
	D3D12_DESCRIPTOR_RANGE range1{};
	range1.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range1.NumDescriptors = UINT_MAX; // ヒープ内のすべてのSRVにアクセスできるように無限（UINT_MAX）にする！
	range1.BaseShaderRegister = 0;
	range1.RegisterSpace = 1; // space1 に配置する
	range1.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // テクスチャはPixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = &range1;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

	// Slot3 : バインドレスヒープ全体を指すディスクリプタテーブル (t0, space2)
	// レンジの設定
	D3D12_DESCRIPTOR_RANGE range2{};
	range2.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range2.NumDescriptors = UINT_MAX; // ヒープ内のすべてのSRVにアクセスできるように無限（UINT_MAX）にする！
	range2.BaseShaderRegister = 0;
	range2.RegisterSpace = 2; // space2 に配置する
	range2.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // テクスチャはPixelShaderで使う
	rootParameters[3].DescriptorTable.pDescriptorRanges = &range2;
	rootParameters[3].DescriptorTable.NumDescriptorRanges = 1;

	// --- 静的サンプラーの設定（s0, space0） --- //
	D3D12_STATIC_SAMPLER_DESC staticSampler{};
	staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSampler.MaxLOD = D3D12_FLOAT32_MAX;
	staticSampler.ShaderRegister = 0;
	staticSampler.RegisterSpace = 0;
	staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// --- ルートシグネチャのビルド --- //
	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	desc.pParameters = rootParameters;
	desc.NumParameters = _countof(rootParameters);
	desc.pStaticSamplers = &staticSampler;
	desc.NumStaticSamplers = 1;

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

	hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	assert(SUCCEEDED(hr));

	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(rootSignature_.GetAddressOf()));
	assert(SUCCEEDED(hr));
}