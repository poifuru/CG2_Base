#pragma once
#include "BasePostEffect.h"

class Dissolve : public BasePostEffect {
public:
	void Initialize(DxCommon* dxCommon) override;

	void Draw(RenderTexture* renderTexture, CameraOrganizer* camera) override;

	void ImGui() override;

private:
	struct alignas(16) DissolveForGPU {
		Vector4 edgeColor;
		float threshold;
		float edgeWidth;
		float padding[2];
	};

private:
	DissolveForGPU* cpuData_ = nullptr;
	TextureData* maskTexture_ = nullptr;
};
