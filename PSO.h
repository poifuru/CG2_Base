#pragma once
#include "header.h"
#include "externals.h"
#include "function.h"

//共通のインターフェースを提供する基底クラス
//これを継承させてGraphicsPipelineStateやComputePipelineStateでポリモーフィズムを使えるようにする
//class PipelineState {
//public:
//	virtual ~PipelineState() = default;
//	virtual void Create(ID3D12Device* device, HRESULT hr,
//		IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler,
//		IDxcIncludeHandler* includeHandler, std::ofstream& os) = 0;
//
//	ID3D12PipelineState* Get() const { return pipelineState.Get(); }
//
//protected:
//	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
//};

class PipelineStateObject {
public:
	//コンストラクタ
	PipelineStateObject(ID3D12Device* device, HRESULT hr, IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler,
		IDxcIncludeHandler* includeHandler, std::ofstream& os);
	
	//デストラクタ
	~PipelineStateObject();

	//実際にPSOを生成する
	void Generate(ID3D12Device* device, HRESULT hr);


	/*void SetRootSignature(ID3D12Device* device, HRESULT hr, std::ofstream& os);

	void SetInputLayout();

	void SetBlendState();

	void SetRasterizerState();

	void SetShaders(IDxcUtils* idxcUtils, IDxcCompiler3* dxcCompiler, 
		IDxcIncludeHandler* includeHandler, std::ofstream& os);

	void Create(ID3D12Device* device, HRESULT hr,
		IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler,
		IDxcIncludeHandler* includeHandler, std::ofstream& os) override;*/

public:
	//ルートシグネチャーのディスクリプタ
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature_{};
	ID3DBlob* signatureBlob_;
	ID3DBlob* errorBlob_;
	ID3D12RootSignature* rootSignature_;

	//InputLayout用の変数
	D3D12_INPUT_ELEMENT_DESC inputElementDescs_[2]{};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_{};

	//ブレンドの設定
	D3D12_BLEND_DESC blendDesc_{};

	//RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc_{};

	//shaderのコンパイル
	IDxcBlob* vertexShaderBlob_;
	IDxcBlob* pixelShaderBlob_;

	//グラフィックスパイプラインのディスクリプタ
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc_{};
	ID3D12PipelineState* graphicsPipelineState_;

	//DiscriptorRange
	D3D12_DESCRIPTOR_RANGE descriptorRange_[1]{};

	//RootParameter作成。複数設定できるので配列。今回は結果1つだけなので長さ1の配列
	D3D12_ROOT_PARAMETER rootParameter_[3]{};

	//Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers_[1]{};

	//DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc_{};

	//DepthStencilTextureをウィンドウサイズで作成
	ID3D12Resource* depthStencilResource_;

	//DSV用のヒープでディスクリプタの数は1。DSVはShader内で触るものではないので、ShaderVisibleはfalse
	ID3D12DescriptorHeap* dsvDescriptorHeap_;

	//depthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};
};

