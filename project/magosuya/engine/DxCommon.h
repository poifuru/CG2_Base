#pragma once
#include <Windows.h>
#include <Wrl.h>
using namespace Microsoft::WRL;
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include <dxgidebug.h>
#include <array>
#include <string>
#include <chrono>
#include <DirectXTex.h>
#include "WindowsAPI.h"
#include "LeakChecker.h"
#include <memory>

class SRVManager;
class RenderTexture;

class DxCommon {
public:		//メンバ関数(mainで呼び出すよう)
	static DxCommon* GetInstance() {
		//初めて呼び出されたときに一回だけ初期化
		static DxCommon instance;
		return &instance;
	}

	void Initialize();
	void InitializeRenderTexture(SRVManager* srvManager);
	void BeginFrame();
	void PreDrawImGui();
	void EndFrame();
	void Finalize() const;

	RenderTexture* GetRenderTexture() const { return renderTexture_.get(); }

	/// <summary>
	/// Resource作成関数
	/// </summary>
	/// <param name="sizeInBytes"></param>
	/// <returns></returns>
	ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	/// <summary>
	/// ディスクリプタヒープ作成関数
	/// </summary>
	/// <param name="device"></param>
	/// <param name="heapType"></param>
	/// <param name="numDescriptors"></param>
	/// <param name="shaderVisible"></param>
	/// <returns></returns>
	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	// RTVの空きハンドルを渡す関数
	D3D12_CPU_DESCRIPTOR_HANDLE AllocateRTV();

	// バックバッファのハンドルとリソース取得
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferRtvHandle() {
		UINT index = swapChain_->GetCurrentBackBufferIndex();
		return rtvHandles_[index];
	}
	ID3D12Resource* GetCurrentBackBufferResource() {
		UINT index = swapChain_->GetCurrentBackBufferIndex();
		return swapChainResources_[index].Get();
	}

private:
	//コンストラクタを禁止
	DxCommon();
	~DxCommon(); // unique_ptrのデストラクタ解決のため明示的に宣言
	// コピーコンストラクタと代入演算子を禁止
	DxCommon(const DxCommon&) = delete;
	DxCommon& operator=(const DxCommon&) = delete;
	DxCommon(DxCommon&&) = delete;
	DxCommon& operator=(DxCommon&&) = delete;

private:	//プライベート関数
	void InitializeFixFPS();
	void CreateDevice();
	void CreateCommand();
	void CreateDxcCompiler();
	void CreateFence();
	void CreateDescriptorHeap();
	void CreateSwapChain();
	void CreateDepthBuffer();
	void CreateRTV();
	void CreateDSV();
	void ViewportRectInit();
	void ScissorRectInit();
	void UpdateFixFPS();

public:		//アクセッサ
	ID3D12Device* GetDevice() { return device_.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() { return commandList_.Get(); }
	IDxcUtils* GetDxcUtils() { return dxcUtils_.Get(); }
	IDxcCompiler3* GetDxcCompiler() { return dxcCompiler_.Get(); }
	IDxcIncludeHandler* GetIncludeHandler() { return includeHandler_.Get(); }

private://メンバ変数
	LeakChecker leakCheck_{};

	//FPS固定用
	std::chrono::steady_clock::time_point reference_;

	//***DX12変数***//
	//DXGIファクトリー
	ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;

	//使用するアダプタ用の変数。最初にnullptrを入れておく
	ComPtr<IDXGIAdapter4> useAdapter_ = nullptr;

	//dxcCompiler
	ComPtr<IDxcUtils> dxcUtils_ = nullptr;
	ComPtr<IDxcCompiler3> dxcCompiler_ = nullptr;

	//後のincludeに対応するための設定
	ComPtr<IDxcIncludeHandler> includeHandler_ = nullptr;

	//デバイス
	ComPtr<ID3D12Device> device_ = nullptr;

	//コマンドキュー
	ComPtr<ID3D12CommandQueue> commandQueue_ = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc_{};

	//コマンドアロケータ
	ComPtr<ID3D12CommandAllocator> commandAllocator_ = nullptr;

	//コマンドリスト
	ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;

	//スワップチェーン
	ComPtr<IDXGISwapChain4> swapChain_ = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};

	//swapChainResource
	std::array<ComPtr<ID3D12Resource>, 2> swapChainResources_;

	// RTVの最大数
	static inline const uint32_t kMaxRTVNum_ = 32;

	// スワップチェーンで使うRTVの個数
	static inline const uint32_t kSwapChainNum_ = 2;

	// RTVの次の空きインデックスを管理する変数
	uint32_t rtvIndex_ = 0;

	//RTVを最大数分作ためのディスクリプタを用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[kMaxRTVNum_]{};

	//ディスクリプタヒープサイズ
	UINT rtvDescriptorHeapSize_;

	//RTVディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_{};

	//RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};

	//初期値0でFenceを作る
	ComPtr<ID3D12Fence> fence_ = nullptr;
	uint64_t fenceValue_ = 0;

	//FenceのSignalを待つためのイベント
	HANDLE fenceEvent_;

	//DepthStencilTexture
	ComPtr<ID3D12Resource> depthStencilResource_ = nullptr;

	//ディスクリプタヒープサイズ
	UINT dsvDescriptorHeapSize_;

	//DSV用ディスクリプタヒープで数は1。DSVはShader内で触るものではないので、ShaderVisibleはfalse
	ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_{};

	//ビューポート
	D3D12_VIEWPORT viewport_{};

	//シザー矩形
	D3D12_RECT scissorRect_{};

	//オフスクリーン描画用
	std::unique_ptr<RenderTexture> renderTexture_ = nullptr;
};