#pragma once
#include "../../header/Engine.h"
#include "../utility/function.h"
#include "../camera/DebugCamera.h"
#include "../2d/Sprite.h"
#include "../3d/sphereModel.h"
#include "../../header/struct.h"
#include "../utility/Math.h"
#include "../3d/Model.h"
#include "../../externals.h"
#include "../../Utility/Shape/Shape.h"
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")
#include <Xinput.h>
#pragma comment(lib, "xinput.lib")
#include <format>// C++20のformat() 文字列整形
#include <chrono>	//時間を扱うライブラリ
#include <sstream>// stringstream

struct D3DResourceLeakChecker {
	~D3DResourceLeakChecker () {
		ComPtr<IDXGIDebug> debug;
		if (SUCCEEDED (DXGIGetDebugInterface1 (0, IID_PPV_ARGS (debug.GetAddressOf ())))) {
			debug->ReportLiveObjects (DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects (DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects (DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		}
	}
};

class DxCommon {
public:
	void Initialize ();

	void BeginFrame ();
	void EndFrame ();

	void Finalize ();

	Shape shape;

	ComPtr<ID3D12Device> GetDevice () { return device; }
	ComPtr<ID3D12GraphicsCommandList> GetCommandList () { return commandList; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureHandle () { return textureSrvHandleGPU; }

private:
	D3DResourceLeakChecker leakCheck;

	//クライアント領域のサイズ
	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;

	ComPtr<IDXGISwapChain4> swapChain = nullptr;
	//DXGIファクトリーの生成
	ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	//使用するアダプタ用の変数。最初にnullptrを入れておく
	ComPtr<IDXGIAdapter4> useAdapter = nullptr;
	//dxcCompilerを初期化
	ComPtr<IDxcUtils> dxcUtils = nullptr;
	ComPtr<IDxcCompiler3> dxcCompiler = nullptr;
	//後のincludeに対応するための設定を作る
	ComPtr<IDxcIncludeHandler> includeHandler = nullptr;
	//デバイス生成
	ComPtr<ID3D12Device> device = nullptr;
	//コマンドキューを生成する
	ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	//コマンドアロケータを生成する
	ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	//コマンドリストを生成する
	ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	//ディスクリプタヒープの生成
	ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	//SRV用のヒープでディスクリプタの数は128。SRVはShader内で触るものなので、ShaderVisibleはtrue
	ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;
	//swapChainからResourceを引っ張ってくる
	ComPtr<ID3D12Resource> swapChainResources[5] = { nullptr };
	//初期値0でFenceを作る
	ComPtr<ID3D12Fence> fence = nullptr;
	//シリアライズしてバイナリにする
	ComPtr<ID3DBlob> signatureBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	//バイナリを元に作成
	ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	//Shaderをコンパイルする
	ComPtr<IDxcBlob> vertexShaderBlob;
	ComPtr<IDxcBlob> pixelShaderBlob;
	//DepthStencilTextureをウィンドウサイズで作成
	ComPtr<ID3D12Resource> depthStencilResource;
	//DSV用のヒープでディスクリプタの数は1。DSVはShader内で触るものではないので、ShaderVisibleはfalse
	ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;
	//実際に生成
	ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	//平行光源のResourceを作成してデフォルト値を書き込む
	ComPtr<ID3D12Resource> textureResource0;
	ComPtr<ID3D12Resource> intermediateResource0;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	uint64_t fenceValue = 0;
	//FenceのSignalを待つためのイベントを作成する
	HANDLE fenceEvent;
	//ウィンドウを生成
	HWND hwnd;
	D3D12_VIEWPORT viewport{};
	D3D12_RECT scissorRect{};



	//頂点データ
	ComPtr<ID3D12Resource> vertexBuffer_;
	std::vector<VertexData> vertexData_;
	VertexData* vertexDataPtr_;
	D3D12_VERTEX_BUFFER_VIEW vbView_{};

	//インデックスデータ
	ComPtr<ID3D12Resource> indexBuffer_;
	D3D12_INDEX_BUFFER_VIEW ibView_{};

	//行列データ
	ComPtr<ID3D12Resource> materialBuffer_;
	TransformationMatrix* matrixData_;
	Transform transform_;	//ローカル座標
	Transform uvTransform_;	//uvTranform用のローカル座標
	TransformationMatrix transformationMatrix_; //ワールド座標と、カメラからwvp行列をもらって格納する

	//マテリアルデータ
	ComPtr<ID3D12Resource> matrixBuffer_;
	Material* materialData_;

	ComPtr<ID3D12Resource> dierctionalLightResource;
	DirectionalLight* lightData;
};

