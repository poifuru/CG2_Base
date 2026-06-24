#include "PCH.h"
#include "Material.h"
#include "GraphicsDevice.h"
#include "DescriptorHeapManager.h"
#include "MathFunction.h"

void Material::Initialize(GraphicsDevice* device, DescriptorHeapManager* heapManager) {
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
	if (isDirty_ && buffer_) {
		// 送信する構造体データの中に行列を計算して詰める
		data_.uvTransform = Math::MakeAffineMatrix(
			uvTransform_.scale, uvTransform_.rotate, uvTransform_.translate
		);

		buffer_->Update({ data_ });
		isDirty_ = false;
	}
}