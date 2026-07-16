#pragma once
#include "MeshRendererComponent.h"
#include "ConstantBuffer.h"

class WaterSurfaceComponent : public MeshRendererComponent {
public:
	void Initialize() override;

	void Update() override;

	void ImGui() override;

	void Serialize(json& j) const override;
	void Deserialize(const json& j) override;

	const char* GetName() const override { return "Water Surface"; }

	D3D12_GPU_VIRTUAL_ADDRESS GetCustomBufferAddress() const {
		return waterSurfaceBuffer_.GetGPUVirtualAddress();
	}

private:
	// 波のパラメータ
	struct WaterSurfaceInfo {
		float amplitude;				// 振幅(A)
		float frequency;				// 周波数(w)
		float steepness;				// 険しさ(Q)
		float padding;
		Vector2 direction;				// 波の進行方向(D)
		float padding2[2];    
	};

	// シェーダーに送るパラメータ
	struct WaterSurfaceForGPU {
		WaterSurfaceInfo waves[4];
		float time;
		int numActiveWaves;
		float padding[2];
	};

	float time_ = 0.0f;

	// ゲルストナー波のパラメータ
	WaterSurfaceInfo waves_[4];
	int numActiveWaves_ = 2; // デフォルトで有効にする波の数（1〜4）

	ConstantBuffer<WaterSurfaceForGPU> waterSurfaceBuffer_;
};