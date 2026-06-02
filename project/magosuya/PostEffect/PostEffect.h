#pragma once
#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;
#include "PSOManager.h"

class DxCommon;
class RenderTexture;

struct PostProcessData {
	float intensity;     // エフェクトの強度（0.0 〜 1.0など）
	float time;          // 時間（ノイズのアニメーションや画面の揺れに使う）
	float dummy1;        // 16バイトアライメント用のパディング
	float dummy2;
};

class PostEffect {
public:
	static PostEffect* GetInstance() {
		static PostEffect instance;
		return &instance;
	}

	void Initialize(DxCommon* dxCommon);

	void Draw(RenderTexture* renderTexture);

	void Imgui();

private:
	PostEffect() = default;
	~PostEffect() = default;
	PostEffect(const PostEffect&) = delete;
	PostEffect& operator=(const PostEffect&) = delete;

	DxCommon* dxCommon_ = nullptr;
	PSODescriptor psoDesc_{};

	// パラメータ変更用変数
	ComPtr<ID3D12Resource> postProcessResource_ = nullptr;
	PostProcessData* postProcessData_ = nullptr;
};