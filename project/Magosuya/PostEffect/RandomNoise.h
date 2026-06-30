#pragma once
#include "BasePostEffect.h"

class RandomNoise : public BasePostEffect {
public:
	void Initialize(DxCommon* dxCommon) override;
	void Draw(RenderTexture* renderTexture, CameraOrganizer* camera) override;
	void ImGui() override;

private:
	// GPUに送る定数バッファ用の構造体
	struct alignas(16) RandomNoiseForGPU {
		float time;
		float intensity;
		float padding[2];
	};

private:
	RandomNoiseForGPU* cpuData_ = nullptr;
	uint32_t srvIndexSceneColor_ = 0; // 入力テクスチャ用SRVインデックス
};