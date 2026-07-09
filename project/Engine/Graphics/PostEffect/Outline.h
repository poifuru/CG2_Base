#pragma once
#include "BasePostEffect.h"

class Outline : public BasePostEffect {
public:
	void Initialize(DxCommon* dxCommon) override;

	void Draw(RenderTexture* renderTexture, CameraOrganizer* camera) override;

	void ImGui() override;

private:
	struct alignas(16) OutlineForGPU {
		Vector4 color = { 0.0f, 0.0f, 0.0f, 1.0f }; // 初期値：黒色不透明
		float edgeThreshold = 0.01f;                 // 初期値：感度
		float edgeThickness = 1.0f;                  // 初期値：太さ1倍
		float cameraNear = 0.1f;
		float cameraFar = 1000.0f;
		float padding[3];
	};

private:
	OutlineForGPU* cpuData_ = nullptr;

	uint32_t srvIndexSceneColor_ = 0; // シーンカラー用のインデックス
	uint32_t srvIndexDepth_ = 0;      // デプスバッファ用のインデックス
};

