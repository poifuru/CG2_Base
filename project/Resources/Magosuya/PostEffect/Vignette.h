#pragma once
#include "BasePostEffect.h"

class Vignette : public BasePostEffect {
public:
	void Initialize(DxCommon* dxCommon) override;

	void Draw(RenderTexture* renderTexture, CameraOrganizer* camera) override;

	void ImGui() override;

private:
	struct alignas(16) VignetteForGPU {
		Vector4 centerAndRadius;      // xy: center, z: innerRadius, w: outerRadius
		Vector4 colorAndIntensity;    // xyz: color, w: intensity
		Vector4 aspectAndPadding;     // x: aspectRatio, yzw: padding
	};

private:
	VignetteForGPU* cpuData_ = nullptr;
};