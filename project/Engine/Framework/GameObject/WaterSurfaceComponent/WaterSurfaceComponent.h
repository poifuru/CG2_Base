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
	// シェーダーに送るパラメータ
	struct WaterSurfaceInfo {
		float amplitude;
		float frequency;
		float steepness;
		float time;
		Vector2 direction;
	};

	float time_ = 0.0f;

	// ゲルストナー波のパラメータ
	float amplitude_ = 0.5f;				// 振幅(A)
	float frequency_ = 0.5f;				// 周波数(w)
	float steepness_ = 0.5f;				// 険しさ(Q)
	Vector2 direction_ = { 1.0f, 1.0f };	// 波の進行方向(D)

	ConstantBuffer<WaterSurfaceInfo> waterSurfaceBuffer_;
};