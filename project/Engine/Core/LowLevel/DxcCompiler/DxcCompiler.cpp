#include "PCH.h"
#include "DxcCompiler.h"

void MyEngine::LowLevel::DxcCompiler::Initialize() {
	// DXCコンパイラの初期化
	HRESULT hr = S_OK;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(dxcUtils_.GetAddressOf()));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(dxcCompiler_.GetAddressOf()));
	assert(SUCCEEDED(hr));
	hr = dxcUtils_->CreateDefaultIncludeHandler(includeHandler_.GetAddressOf());
	assert(SUCCEEDED(hr));
}