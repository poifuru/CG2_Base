#pragma once
#include <vector>
#include <string>
#include "DxCommon.h"
#include "Animation.h"
#include "Skeleton.h"
#include "SkinCluster.h"
#include "Model.h"

class Animator {
public:
	Animator(DxCommon* dxCommon);
	~Animator();

	// スキニング用の初期化（モデルデータからスケルトンやスキンクラスターを構築）
	void Initialize(Model* targetModel);

	// アニメーションの切り替え
	void BindAnimation(Animation* animation);

	// 毎フレームの骨の計算
	void Update();

	// ゲッター群（レンダラーが描画するときに引っこ抜く）
	Animation* GetAnimation() const { return animation_; }
	bool IsAnimation() const { return animation_->isAnimation; }
	void SetIsAnimation(bool flag) { animation_->isAnimation = flag; }
	bool IsSkinning() const { return isSkinning_; }
	void SetIsSkinning(bool flag) { isSkinning_ = flag; }
	const SkinClusterResource& GetSkinCluster() const { return skinCluster_; }

	// Modelがワールド行列を作るときに呼び出すルートの行列
	Matrix4x4 GetRootAnimationMatrix() const { return rootAnimationMatrix_; }

	// スキニング用の頂点バッファビューを取得
	D3D12_VERTEX_BUFFER_VIEW GetInfluenceVBV() { return skinCluster_.GetInfluenceView(); }

private:
	void AnimationTimeUpdate();
	Skeleton CreateSkeleton(const Node& roodNode);
	int32_t CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints);
	void AnimationUpdate();
	void EvaluateAnimation();
	void CreateSkinCluster(ModelData* modelData);
	void SkeletonUpdate();

private:
	DxCommon* dxCommon_ = nullptr;
	Model* targetModel_ = nullptr; // 紐づくモデルのポインタ

	// ルートノードのアニメーション結果を保持するメンバ
	Matrix4x4 rootAnimationMatrix_{};

	// アニメーション状態（Modelから完全に追い出された！）
	Animation* animation_ = nullptr;

	// スキニング状態
	bool isSkinning_ = false;
	Skeleton skeleton_ = {};
	SkinClusterResource skinCluster_ = {};
	SkinClusterData skinClusterData_;
};