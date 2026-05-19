#pragma once
#include <Windows.h>
#include <Wrl.h>
using namespace Microsoft::WRL;
#include <d3d12.h>
#include <vector>
#include <string>
#include "struct.h"
#include "PSOManager.h"
#include "DxCommon.h"
#include "MaterialData.h"

class LightManager;

class ModelRenderer {
public:
	ModelRenderer (DxCommon* dxCommon, LightManager* lightManager);
	~ModelRenderer ();

	void Initialize ();
	void Update (Matrix4x4 world, Matrix4x4 vp, EulerTransform uvTransform, Vector3 cameraWorld);
	void Draw (D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);
	void ImGui (EulerTransform& transform, EulerTransform& uvTransform, const std::string& windowName);

	//アクセッサ
	MaterialData* GetMaterial () { return materialData_; }
	void IsLighting (const BOOL& flag) { materialData_->enableLighting = flag; }
	void SetColor (const Vector4& color) { materialData_->color = color; }
	void SetAlpha(const float& alpha) { materialData_->color.w = alpha; }
	void SetRoughness(const float& roughness) { materialData_->roughness = roughness; }
	void SetMetallic(const float& metallic) { materialData_->metallic = metallic; }
	void SetImGuiID (const std::string& id) { tag_ = id; }
	void SetMeshData (const std::weak_ptr<ModelData>& data){ modelData_ = data; }
	void SetDepthEnable(bool flag) { desc_.DepthEnable = flag; }

	/// <summary>
	/// 使うAnimationをセット
	/// </summary>
	/// <param name="animation">ロードしたAnimation</param>
	void SetAnimation(const std::weak_ptr<Animation>& animation) { animationData_ = animation; }

	void SkeletonInit();

private: // 内部関数
	Matrix4x4 AnimationUpdate(ModelData* modelData);
	Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time);
	Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time);
	Skeleton CreateSkeleton(const Node& roodNode);
	int32_t CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints);
	void SkeletonUpdate(Skeleton& skeleton);
	void ApplyAnimation();
	void DrawSkeleton();
	SkinCluster CreateSkinCluster(std::shared_ptr<ModelData>& modelData);
	void SkinClusterUpdate();

private:
	// モデルデータ
	std::weak_ptr<ModelData> modelData_;
	// アニメーションデータ
	std::weak_ptr<Animation> animationData_;
	float animationTime_ = 0.0f;
	Matrix4x4 animationLocalMat_ = {};

	// スケルトンデータ
	Skeleton skeleton_ = {};
	// スケルトンデバッグ描画用
	Matrix4x4 vp_ = {};
	Matrix4x4 world_ = {};

	// スキンクラスター
	SkinCluster skinCluster_ = {};
	// Skinning用のSRVIndex
	uint32_t DescriptorFreeIndex_ = 0;

	//PSO
	PSODescriptor desc_ = {};

	//GPUリソース
	ComPtr<ID3D12Resource> matrixBuffer_;
	ComPtr<ID3D12Resource> materialBuffer_;
	ComPtr<ID3D12Resource> cameraBuffer_;

	//GPUリソースにマッピングするデータ
	TransformationMatrix* matrixData_ = nullptr;
	Material* materialData_ = nullptr;
	Vector3* cameraData_ = nullptr;

	//ImGui用のラベル名
	std::string tag_;
	int instanceID_ = 0;
	static inline int modelCount_ = 0;

	//ImGuiで色をいじる変数
	float color_[4];
	bool drawSkeleton_ = false;

	//ポインタを借りる
	DxCommon* dxCommon_ = nullptr;
	ID3D12Device* device_ = nullptr;
	ID3D12GraphicsCommandList* commandList_ = nullptr;
	LightManager* lightManager_ = nullptr;
};