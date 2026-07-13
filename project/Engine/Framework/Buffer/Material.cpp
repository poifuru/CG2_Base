#include "PCH.h"
#include "Material.h"
#include "GraphicsDevice.h"
#include "DescriptorHeapManager.h"
#include "RootSignatureManager.h"
#include "PSOManager.h"
#include "MathFunction.h"

void Material::Initialize(
	MyEngine::LowLevel::GraphicsDevice* device,
	MyEngine::LowLevel::DescriptorHeapManager* heapManager,
	MyEngine::Rendering::RootSignatureManager* rootSigManager,
	MyEngine::Rendering::PSOManager* psoManager,
	MyEngine::Rendering::ShaderManager* shaderManager,
	MyEngine::Rendering::InputLayoutManager* inputLayoutManager,
	MyEngine::Rendering::BlendModeManager* blendModeManager
) {
	device_ = device;
	rootSigManager_ = rootSigManager;
	psoManager_ = psoManager;
	shaderManager_ = shaderManager;
	inputLayoutManager_ = inputLayoutManager;
	blendModeManager_ = blendModeManager;

	buffer_ = std::make_unique<StructuredBuffer<MaterialData>>();
	buffer_->Initialize(device->GetDevice(), *heapManager, 1);

	// デフォルト値の設定
	data_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	data_.uvTransform = Math::MakeIdentity4x4();
	data_.roughness = 0.5f;
	data_.metallic = 0.0f;
	data_.environmentCoefficient = 0.3f;
	data_.enableLighting = TRUE;

	uvTransform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	isDirty_ = true;
}

void Material::Update() {
	// 値が書き換わっている時だけGPUに送る
	if(isDirty_ && buffer_) {
		// 送信する構造体データの中に行列を計算して詰める
		data_.uvTransform = Math::MakeAffineMatrix(
			uvTransform_.scale, uvTransform_.rotate, uvTransform_.translate
		);

		buffer_->Update({ data_ });
		isDirty_ = false;
	}
}

void Material::SetShader(uint32_t vsID, uint32_t psID) {
	vsID_ = vsID;
	psID_ = psID;
	UpdatePSO(); // 設定が変わったら自動的にPSOを再構築
}

void Material::SetBlendMode(MyEngine::Rendering::BlendModeType blendMode) {
	blendMode_ = blendMode;
    UpdatePSO();
}

void Material::SetDepthEnable(bool enable) {
	isDepthEnable_ = enable;
	UpdatePSO();
}

void Material::SetDoubleSided(bool doubleSided) {
	isDoubleSided_ = doubleSided;
	UpdatePSO();
}

void Material::UpdatePSO() {
	/*if (!device_ || !psoManager_ || vsID_ == 0 || psID_ == 0) return;*/
	if (!device_ || !psoManager_ || vsID_ == 0 || psID_ == 0) {
		return;
	}

	// 高レベル設定から PSODescriptor を組み立てる（描画ループの外で行う！）
	MyEngine::Rendering::PSODescriptor desc{};
	desc.VS_ID = vsID_;
	desc.PS_ID = psID_;
	desc.InputLayoutID = inputLayout_; 
	desc.BlendMode = blendMode_;
	desc.CullMode = isDoubleSided_ ? D3D12_CULL_MODE_NONE : D3D12_CULL_MODE_BACK;
	desc.DepthEnable = isDepthEnable_;
	desc.DepthWriteMask = depthWriteEnable_ ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	// PSOを事前作成してポインタをキャッシュ
	ID3D12RootSignature* rootSig = rootSigManager_->GetCommonRootSignature(); 
	pso_ = psoManager_->GetOrCreatePSO(
		device_->GetDevice(),
		desc,
		rootSig,
		*shaderManager_,
		*inputLayoutManager_,
		*blendModeManager_
	);

	assert(pso_ != nullptr && "PSOの生成自体に失敗して nullptr になりました！");
}