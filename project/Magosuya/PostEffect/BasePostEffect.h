#pragma once
#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;
#include "PSOManager.h"

class DxCommon;
class RenderTexture;

class BasePostEffect {
public:
	virtual ~BasePostEffect() = default;
	virtual void Initialize(DxCommon* dxCommon) = 0;
	virtual void Draw(RenderTexture* renderTexture) = 0;
	virtual void Imgui() = 0;
};