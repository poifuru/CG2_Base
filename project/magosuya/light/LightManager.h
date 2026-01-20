#pragma once
#include <vector>
#include <memory>
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "DxCommon.h"

//ライトの種類
enum LightType {
	DIRECTIONALLIGHT,
	POINTLIGHT,
	SPOTLIGHT,
};

struct LightCount {
	int32_t dirLight;
	int32_t pointLight;
	int32_t spotLight;
};

struct DirectionalLightForGPU {
	Vector4 color;
	Vector3 direction;
	float intensity;
};

struct PointLightForGPU {
	Vector4 color;
	Vector3 position;
	float intensity;
	float radius;	//ライトが届く最大距離
	float decay;	//減衰率
	float padding[2];
};

struct SpotLightForGPU {
	Vector4 color;
	Vector3 position;
	float intensity;
	Vector3 direction;
	float distance;
	float decay;
	float cosAngle;
	float padding[2];
};

class LightManager {
public:
	LightManager(DxCommon* dxCommon);
	~LightManager();

	void Initialize();
	void Update();
	void ImGui();

	// シーンからライトを追加する関数
	void AddLight(LightType type);

	// 描画時にバッファーを渡すための関数
	ID3D12Resource& GetLightCountBuffer() { return *lightCountBuffer_.Get(); }
	uint32_t GetDirLightSrvHandle() const { return dirLightSRVIndex_; }
	uint32_t GetPointLightSrvHandle() const { return pointLightSRVIndex_; }
	uint32_t GetSpotLightSrvHandle() const { return spotLightSRVIndex_; }

private:
	//ライトの種類ごとにリストを持つ
	std::vector<std::unique_ptr<DirectionalLight>> dirLights_;
	std::vector<std::unique_ptr<PointLight>> pointLights_;
	std::vector<std::unique_ptr<SpotLight>> spotLights_;

	//ConstantBuffer用のリソース
	ComPtr<ID3D12Resource> lightCountBuffer_;
	LightCount* lightCountData_ = nullptr;

	//StructuredBuffer用のリソース
	ComPtr<ID3D12Resource> dirLightBuffer_;	//DirectionalLight
	DirectionalLightForGPU* dirLightData_ = nullptr;

	ComPtr<ID3D12Resource> pointLightBuffer_;	//PointLight
	PointLightForGPU* pointLightData_ = nullptr;

	ComPtr<ID3D12Resource> spotLightBuffer_;	//SpotLight
	SpotLightForGPU* spotLightData_ = nullptr;

	// DescriptorHeap内での位置（SRV用）
	uint32_t dirLightSRVIndex_;
	uint32_t pointLightSRVIndex_;
	uint32_t spotLightSRVIndex_;

	//ImGui編集用の変数
	int selectDirLightIndex_ = 0;
	int selectPointLightIndex_ = 0;
	int selectSpotLightIndex_ = 0;

	//ポインタを借りる
	DxCommon* dxCommon_ = nullptr;
};