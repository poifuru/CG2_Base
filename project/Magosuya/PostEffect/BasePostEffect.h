#pragma once
#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;
#include "PSOManager.h"
#include "SRVManager.h"
#include "RenderTexture.h"
#include "struct.h"

class DxCommon;
class CameraOrganizer;

class BasePostEffect {
public:
	virtual ~BasePostEffect() = default;

	// 継承先でPixelShaderを上書きする
	virtual void Initialize(DxCommon* dxCommon) = 0;

	// 継承先でCBVをセットする
	virtual void Draw(RenderTexture* renderTexture, CameraOrganizer* camera) = 0;
	virtual void ImGui() = 0;

	bool GetIsActive() { return isActive_; }
	void SetIsActive(bool flag) { isActive_ = flag; }

protected:
	DxCommon* dxCommon_ = nullptr;
	PSODescriptor psoDesc_{};

	// ポストエフェクトを有効にするかのフラグ
	bool isActive_ = false;

	ComPtr<ID3D12Resource> constantBuffer_ = nullptr;
};