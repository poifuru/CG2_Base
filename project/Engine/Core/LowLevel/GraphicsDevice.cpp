#include "PCH.h"
#include "GraphicsDevice.h"

GraphicsDevice::GraphicsDevice() = default;

void GraphicsDevice::Initialize() {
	HRESULT hr = S_OK;

	// デバッグレイヤーの有効化(Debug, Development時)
#ifdef USEIMGUI
	EnableDebugLayer();
#endif

	// DXGIファクトリーの生成
	hr = CreateDXGIFactory2(0, IID_PPV_ARGS(dxgiFactory_.GetAddressOf()));
	assert(SUCCEEDED(hr));

	// 最適なハードウェアアダプタの選定
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter;
	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(useAdapter.GetAddressOf())) != DXGI_ERROR_NOT_FOUND; ++i) {
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));

		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) {
			break; // ハードウェアグラフィックカードが見つかったら確定
		}
		useAdapter = nullptr;
	}
	assert(useAdapter != nullptr);

	// D3D12デバイスの生成(12.0以上を要求)
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0 };
	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(device_.GetAddressOf()));
		if (SUCCEEDED(hr)) {
			break;
		}
	}
	assert(device_ != nullptr);

	// エラー・警告の出力制御(Debug, Developmentのみ)
#ifdef USEIMGUI
	SetupInfoQueue();
#endif

	// DXCコンパイラの初期化
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(dxcUtils_.GetAddressOf()));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(dxcCompiler_.GetAddressOf()));
	assert(SUCCEEDED(hr));
	hr = dxcUtils_->CreateDefaultIncludeHandler(includeHandler_.GetAddressOf());
	assert(SUCCEEDED(hr));
}

void GraphicsDevice::EnableDebugLayer() {
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController;
	if(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf())))) {
		debugController->EnableDebugLayer();
		debugController->SetEnableGPUBasedValidation(true);
	}
}

void GraphicsDevice::SetupInfoQueue() {
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue;
	if(SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(infoQueue.GetAddressOf())))) {
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);

		// Windows11等でのバグによる既知のメッセージを抑制
		D3D12_MESSAGE_ID denyIds[] = { D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE };
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };

		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;

		infoQueue->PushStorageFilter(&filter);
	}
}