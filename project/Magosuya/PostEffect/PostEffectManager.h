#pragma once
#include <memory>
#include "BasePostEffect.h"

class DxCommon;
class RenderTexture;

// ポストエフェクトの種類
enum class PostEffectType {
	ColorGrading,
	Vignette,

	Count	// エフェクトの総数
};

class PostEffectManager {
public:
	static PostEffectManager* GetInstance() {
		static PostEffectManager instance;
		return &instance;
	}

	// 初期化時に画面サイズに合わせた中間テクスチャを二枚生成する
	void Initialize(DxCommon* dxCommon, uint32_t windowWidth, uint32_t windowHeight);
	void Finalize();

	void SetEffectActive(PostEffectType type, bool flag);
	void ClearEffects();

	// ピンポンレンダリングを実行する関数
	void Execute(RenderTexture* srcTexture);

	void ImGui();

private:
	PostEffectManager() = default;
	~PostEffectManager() = default;
	PostEffectManager(const PostEffectManager&) = delete;
	PostEffectManager& operator=(const PostEffectManager&) = delete;

private:
	DxCommon* dxCommon_ = nullptr;

	// エフェクトの総数で固定管理する
	std::unique_ptr<BasePostEffect> effects_[static_cast<size_t>(PostEffectType::Count)];

	// ピンポン用の中間テクスチャ2枚
	std::unique_ptr<RenderTexture> workTextures_[2];
};