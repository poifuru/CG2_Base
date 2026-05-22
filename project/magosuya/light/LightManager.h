#pragma once
#include <vector>
#include <memory>
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "RectLight.h"
#include "DxCommon.h"
#include "Buffer.h"

//ライトの最大数
const uint32_t MaxCount = 20;

//ライトの種類
enum LightType {
	DIRECTIONALLIGHT,
	POINTLIGHT,
	SPOTLIGHT,
	RECTLIGHT,
};

struct LightCount {
	int32_t dirLight;
	int32_t pointLight;
	int32_t spotLight;
	int32_t rectLight;
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

struct RectLightForGPU {
	Vector4 color;
	Vector3 position;    // ライトの中心座標
	float intensity;
	Vector3 direction;   // ライトの正面方向（法線）
	float padding1;
	Vector2 size;        // Width(幅) と Height(高さ)
	float padding2[2];
	Vector3 right;       // ライトの右方向ベクトル
	float padding;
	Vector3 up;          // ライトの上方向ベクトル
	float decay;        // 距離による減衰率（PointLightと同様）
};

// すべてのライトをまとめる構造体
struct AllLightDataForGPU {
	LightCount count;

	DirectionalLightForGPU dirLights[MaxCount];
	PointLightForGPU pointLights[MaxCount];
	SpotLightForGPU spotLights[MaxCount];
	RectLightForGPU rectLights[MaxCount];
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
	//ライトを消す関数
	void DeleteLight(LightType type, uint32_t index);

	//---ライトの設定---//
	//***DirectionalLight***//
	void SetDirectionalLightColor(uint32_t index, const Vector4& color) { dirLights_[index]->SetColor(color); }
	void SetDirectionalLightIntensity(uint32_t index, const float& intensity) { dirLights_[index]->SetIntensity(intensity); }
	void SetDirectionalLightDir(uint32_t index, const Vector3& dir) { dirLights_[index]->SetDirection(dir); }

	//***PointLight***//
	void SetPointLightColor(uint32_t index, const Vector4& color) { pointLights_[index]->SetColor(color); }
	void SetPointLightIntensity(uint32_t index, const float& intensity) { pointLights_[index]->SetIntensity(intensity); }
	void SetPointLightPosition(uint32_t index, const Vector3& position) { pointLights_[index]->SetPosition(position); }
	void SetPointLightRadius(uint32_t index, const float& radius) { pointLights_[index]->SetRadius(radius); }
	void SetPointLightDecay(uint32_t index, const float& decay) { pointLights_[index]->SetDecay(decay); }

	//***spotLight***//
	void SetSpotLightColor(uint32_t index, const Vector4& color) { spotLights_[index]->SetColor(color); }
	void SetSpotLightIntensity(uint32_t index, const float& intensity) { spotLights_[index]->SetIntensity(intensity); }
	void SetSpotLightPosition(uint32_t index, const Vector3& position) { spotLights_[index]->SetPosition(position); }
	void SetSpotLightDir(uint32_t index, const Vector3& dir) { spotLights_[index]->SetDirection(dir); }
	void SetSpotLightDistance(uint32_t index, const float& distance) { spotLights_[index]->SetDistance(distance); }
	void SetSpotLightDecay(uint32_t index, const float& decay) { spotLights_[index]->SetDecay(decay); }
	void SetSpotLightCosAngle(uint32_t index, const float& cosAngle) { spotLights_[index]->SetCosAngle(cosAngle); }

	//***RectLight***//
	void SetRectLightColor(uint32_t index, const Vector4& color) { rectLights_[index]->SetColor(color); }
	void SetRectLightIntensity(uint32_t index, const float& intensity) { rectLights_[index]->SetIntensity(intensity); }
	void SetRectLightPosition(uint32_t index, const Vector3& position) { rectLights_[index]->SetPosition(position); }
	void SetRectLightDir(uint32_t index, const Vector3& dir) { rectLights_[index]->SetDirection(dir); }
	void SetRectLightSize(uint32_t index, const Vector2& size) { rectLights_[index]->SetSize(size); }
	void SetRectLightRight(uint32_t index, const Vector3& right) { rectLights_[index]->SetRight(right); }
	void SetRectLightUp(uint32_t index, const Vector3& up) { rectLights_[index]->SetRight(up); }
	void SetRectLightDecay(uint32_t index, const float& decay) { rectLights_[index]->SetDecay(decay); }

	// 描画時にバッファーを渡すための関数
	/*ID3D12Resource& GetLightCountBuffer() { return *lightCountBuffer_.Get(); }
	uint32_t GetDirLightSrvHandle() const { return dirLightSRVIndex_; }
	uint32_t GetPointLightSrvHandle() const { return pointLightSRVIndex_; }
	uint32_t GetSpotLightSrvHandle() const { return spotLightSRVIndex_; }
	uint32_t GetRectLightSrvHandle() const { return rectLightSRVIndex_; }*/
	D3D12_GPU_VIRTUAL_ADDRESS GetLightGPUAddress() { return lightBuffer_.GetGPUVirtualAddress(); }

private:
	//ライトの種類ごとにリストを持つ
	std::vector<std::unique_ptr<DirectionalLight>> dirLights_;
	std::vector<std::unique_ptr<PointLight>> pointLights_;
	std::vector<std::unique_ptr<SpotLight>> spotLights_;
	std::vector < std::unique_ptr<RectLight>> rectLights_;

	ConstantBuffer<AllLightDataForGPU> lightBuffer_;
	AllLightDataForGPU lightCPUData_{}; // CPU側の一時保存用

	//ImGui編集用の変数
	int selectDirLightIndex_ = 0;
	int selectPointLightIndex_ = 0;
	int selectSpotLightIndex_ = 0;
	int selectRectLightIndex_ = 0;

	//ポインタを借りる
	DxCommon* dxCommon_ = nullptr;
};