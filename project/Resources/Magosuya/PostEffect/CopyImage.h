#pragma once
#include "BasePostEffect.h"

class CopyImageEffect : public BasePostEffect {
public:
	void Initialize(DxCommon* dxCommon) override;
	void Draw(RenderTexture* renderTexture, CameraOrganizer* camera) override;
	void ImGui() override;
};
