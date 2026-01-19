#pragma once
#include <vector>
#include <memory>
#include "DirectionalLight.h"
#include "DxCommon.h"

//ライトの種類
enum LightType {
	DIRECTIONALLIGHT,
	POINTLIGHT,
	SPOTLIGHT,
};

struct DirectionalLightData {
	Vector4 color;
	Vector3 direction;
	float intensity;
};

class LightManager {
public:
	LightManager();
	~LightManager();

	void Initialize();
	void Update();

	// シーンからライトを追加する関数
	void AddLight(LightType type);

	// 描画時にSRVのハンドルを渡すための関数
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandle() const { return srvHandleGPU_; }
	uint32_t GetNumDirLights() const { return static_cast<uint32_t>(dirLights_.size()); }

private:
	//ライトの種類ごとにリストを持つ
	std::vector<std::unique_ptr<DirectionalLight>> dirLights_;

	//StructuredBuffer用のリソース
	ComPtr<ID3D12Resource> dirLightBuffer_;
	DirectionalLightData* dirLightData_ = nullptr;

	// DescriptorHeap内での位置（SRV用）
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU_;

	//ポインタを借りる
	DxCommon* dxCommon_ = nullptr;
};