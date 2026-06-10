#pragma once
#include "BasePostEffect.h"

class Vignette : public BasePostEffect {
public:
	void Initialize(DxCommon* dxCommon) override;

	void Draw(RenderTexture* renderTexture) override;

	void ImGui() override;

private:
	struct alignas(16) VignetteForGPU {
		Vector2 center = { 0.5f, 0.5f };	// ビネットの中心座標(基本 0.5, 0.5)
		float innerRadius = 0.0f;			// 暗くなり始める半径 (0.0 ～ 1.0)
		float outerRadius = 0.0f;			// 完全に暗くなる半径 (0.0 ～ 1.0)

		Vector3 vignetteColor {				// ビネットの色(黒なら　0.0, 0.0, 0.0)
			0.0f, 0.0f, 0.0f
		};
		float intensity = 1.0f;				// ビネットの強度(0.0 なら効果なし、1.0 で完全適用)

		float aspectRatio;					// 画面のアスペクト比(Width / Height)
	};

public:
	VignetteForGPU* cpuData_ = nullptr;
};