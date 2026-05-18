#pragma once
#include <d3d12.h>
#include <wrl.h>

class DxCommon;
class RenderTexture;

class PostEffect {
public:
	static PostEffect* GetInstance() {
		static PostEffect instance;
		return &instance;
	}

	void Initialize(DxCommon* dxCommon);

	void Draw(RenderTexture* renderTexture);

private:
	PostEffect() = default;
	~PostEffect() = default;
	PostEffect(const PostEffect&) = delete;
	PostEffect& operator=(const PostEffect&) = delete;

	DxCommon* dxCommon_ = nullptr;
	ID3D12PipelineState* pso_ = nullptr;
	ID3D12RootSignature* rootSignature_ = nullptr;
};
