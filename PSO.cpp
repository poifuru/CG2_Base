#include "PSO.h"

//クライアント領域のサイズ
const int32_t kClientWidth = 1280;
const int32_t kClientHeight = 720;

PipelineStateObject::PipelineStateObject(ID3D12Device* device, HRESULT hr, IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler, std::ofstream& os) {
	//ルートシグネチャーのディスクリプタ
	descriptionRootSignature_.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	descriptionRootSignature_.NumParameters = 0;
	descriptionRootSignature_.pParameters = nullptr;
	descriptionRootSignature_.NumStaticSamplers = 0;
	descriptionRootSignature_.pStaticSamplers = nullptr;
	//シリアライズしてバイナリにする
	signatureBlob_ = nullptr;
	errorBlob_ = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature_,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob_, &errorBlob_);
	if (FAILED(hr)) {
		Log(os, reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
		assert(false);
	}

	//バイナリを元に作成
	rootSignature_ = nullptr;
	hr = device->CreateRootSignature(0, signatureBlob_->GetBufferPointer(),
		signatureBlob_->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));

	//InputLayout用の変数
	inputElementDescs_[0].SemanticName = "POSITION";
	inputElementDescs_[0].SemanticIndex = 0;
	inputElementDescs_[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs_[0].InputSlot = 0;
	inputElementDescs_[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs_[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	inputElementDescs_[0].InstanceDataStepRate = 0;


	inputElementDescs_[1].SemanticName = "TEXCOORD";	
	inputElementDescs_[1].SemanticIndex = 0;
	inputElementDescs_[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs_[1].InputSlot = 0;
	inputElementDescs_[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs_[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	inputElementDescs_[1].InstanceDataStepRate = 0;

	inputLayoutDesc_.pInputElementDescs = inputElementDescs_;
	inputLayoutDesc_.NumElements = _countof(inputElementDescs_);

	//ブレンドの設定
	//すべての色の要素を書き込む
	blendDesc_.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//RasterizerStateの設定
	//裏面(時計回り)を表示しない
	rasterizerDesc_.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	rasterizerDesc_.FillMode = D3D12_FILL_MODE_SOLID;

	//shaderのコンパイル
	vertexShaderBlob_ = CompilerShader(L"Object3d.VS.hlsl",
		L"vs_6_0", dxcUtils, dxcCompiler, includeHandler, os);
	assert(vertexShaderBlob_ != nullptr);

	pixelShaderBlob_ = CompilerShader(L"Object3d.PS.hlsl",
		L"ps_6_0", dxcUtils, dxcCompiler, includeHandler, os);
	assert(pixelShaderBlob_ != nullptr);

	//DiscriptorRange
	descriptorRange_[1] = {};
	descriptorRange_[0].BaseShaderRegister = 0;	//0から始まる
	descriptorRange_[0].NumDescriptors = 1;		//数は1つ
	descriptorRange_[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	//SRVを使う
	descriptorRange_[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;	//Offsetを自動計算

	//RootParameter作成。複数設定できるので配列。今回は結果1つだけなので長さ1の配列
	rootParameter_[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;		//CBVを使う
	rootParameter_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//PixelShaderで使う
	rootParameter_[0].Descriptor.ShaderRegister = 0;
	rootParameter_[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;		//CBVを使う
	rootParameter_[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;	//PixelShaderで使う
	rootParameter_[1].Descriptor.ShaderRegister = 0;						//レジスタ番号とバインド
	descriptionRootSignature_.pParameters = rootParameter_;				//ルートパラメータ配列へのポインタ
	descriptionRootSignature_.NumParameters = _countof(rootParameter_);	//配列の長さ

	//DescriptorTable
	rootParameter_[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;	//DiscriptorTableを使う
	rootParameter_[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//PixelShaderで使う
	rootParameter_[2].DescriptorTable.pDescriptorRanges = descriptorRange_;	//Tableの中身の配列を指定
	rootParameter_[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange_);	//Tableで利用する数

	//Samplerの設定
	staticSamplers_[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;	//バイリニアフィルタ
	staticSamplers_[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers_[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers_[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers_[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;	//比較しない
	staticSamplers_[0].MaxLOD = D3D12_FLOAT32_MAX;	//ありったけのmipmapを使う
	staticSamplers_[0].ShaderRegister = 0;	//レジスタ番号0を使う
	staticSamplers_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;		//PixelShaderで使う
	descriptionRootSignature_.pStaticSamplers = staticSamplers_;
	descriptionRootSignature_.NumStaticSamplers = _countof(staticSamplers_);


	//DepthStencilTextureをウィンドウサイズで作成
	//depthStencilResource_ = CreateDepthStencilTextureResource(device, kClientWidth, kClientHeight);

	//DSV用のヒープでディスクリプタの数は1。DSVはShader内で触るものではないので、ShaderVisibleはfalse
	//dsvDescriptorHeap_ = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	//DSVの設定
	dsvDesc_.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;		//Format。基本的にはResourceに合わせる
	dsvDesc_.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;		//2dTexture
	//DSVHeapの先頭にDSVを作る
	device->CreateDepthStencilView(depthStencilResource_, &dsvDesc_, dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart());

	//Depthの機能を有効化する
	depthStencilDesc_.DepthEnable = true;
	//書き込みします
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}

PipelineStateObject::~PipelineStateObject() {
	signatureBlob_->Release();
	if (errorBlob_) {
		errorBlob_->Release();
	}
	rootSignature_->Release();
	pixelShaderBlob_->Release();
	vertexShaderBlob_->Release();
	graphicsPipelineState_->Release();
}

void PipelineStateObject::Generate(ID3D12Device* device, HRESULT hr) {
	//PSOを生成する
	graphicsPipelineStateDesc_.pRootSignature = rootSignature_;			//RootSignature
	graphicsPipelineStateDesc_.InputLayout = inputLayoutDesc_;			//InputLayout
	graphicsPipelineStateDesc_.VS = { vertexShaderBlob_->GetBufferPointer(),
	vertexShaderBlob_->GetBufferSize() };								//VertexShader
	graphicsPipelineStateDesc_.PS = { pixelShaderBlob_->GetBufferPointer(),
	pixelShaderBlob_->GetBufferSize() };								//PixelShader
	graphicsPipelineStateDesc_.BlendState = blendDesc_;					//BlendState
	graphicsPipelineStateDesc_.RasterizerState = rasterizerDesc_;		//RastarizerState

	//書き込むRTVの情報
	graphicsPipelineStateDesc_.NumRenderTargets = 1;
	graphicsPipelineStateDesc_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトポロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むのかの設定(気にしなくて良い)
	graphicsPipelineStateDesc_.SampleDesc.Count = 1;
	graphicsPipelineStateDesc_.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//DepthStencilの設定
	graphicsPipelineStateDesc_.DepthStencilState = depthStencilDesc_;
	graphicsPipelineStateDesc_.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//実際に生成
	graphicsPipelineState_ = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc_,
		IID_PPV_ARGS(&graphicsPipelineState_));
	assert(SUCCEEDED(hr));
}

//void GraphicsPipelineState::SetRootSignature(ID3D12Device* device, HRESULT hr, std::ofstream& os) {
//	
//}
//
//void GraphicsPipelineState::SetInputLayout() {
//	
//}
//
//void GraphicsPipelineState::SetBlendState() {
//	
//}
//
//void GraphicsPipelineState::SetRasterizerState() {
//	
//}
//
//void GraphicsPipelineState::SetShaders(IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler,
//	
//}
//
//void GraphicsPipelineState::Create(ID3D12Device* device, HRESULT hr,
//	) {
//	SetRootSignature(device, hr, os);
//	SetInputLayout();
//	SetBlendState();
//	SetRasterizerState();
//	SetShaders(dxcUtils, dxcCompiler, includeHandler, os);
//}
