#pragma once
#include "BasePostEffect.h"

class RadialBlur : public BasePostEffect {
public:
	void Initialize(DxCommon* dxCommon) override;

	void Draw(RenderTexture* renderTexture, CameraOrganizer* camera) override;

	void ImGui() override;

private:
	struct alignas(16) RadialBlurForGPU {
		Vector2 center;
		float blurWidth;
		int sampleCount;
	};

private:
	RadialBlurForGPU* cpuData_ = nullptr;
};
