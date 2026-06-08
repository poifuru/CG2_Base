#pragma once
#include <vector>
#include <memory>

class DxCommon;
class RenderTexture;
class BasePostEffect;

class PostEffectManager {
public:
	// ポストエフェクトの種類
	enum class PostEffectType {
		ColorGrading,
		Vignette,
	};

	static PostEffectManager* GetInstance() {
		static PostEffectManager instance;
		return &instance;
	}

	// 初期化時に画面サイズに合わせた中間テクスチャを二枚生成する
	void Initialize(DxCommon* dxCommon, uint32_t windowWidth, uint32_t windowHeight);
	void Finalize();

	void EnableEffect(PostEffectType type);
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
	std::vector<std::unique_ptr<BasePostEffect>> effects_;

	// ピンポン用の中間テクスチャ2枚
	// ※RenderTextureクラスに、内部のリソース（ID3D12Resource*）を取得する 
	// GetResource() や、RTV・SRVハンドルを取得する関数がある前提。
	std::unique_ptr<RenderTexture> workTextures_[2];
};