#pragma once
#include "BasePostEffect.h"

class Fog : public BasePostEffect {
public:
	void Initialize(DxCommon* dxCommon) override;

	void Draw(RenderTexture* renderTexture, CameraOrganizer* camera) override;

	void ImGui() override;

private:
	struct alignas(16) FogForGPU {
		Matrix4x4 inverseVP;
		Vector4 color;
		float start;
		float end;
		float cameraNear;
		float cameraFar;
		float heightStart;
		float heightEnd;
		float padding[2];
	};

private:
	FogForGPU* cpuData_ = nullptr;

	uint32_t srvIndexSceneColor_ = 0; // シーンカラー用のインデックス
	uint32_t srvIndexDepth_ = 0;      // デプスバッファ用のインデックス
};