// =========================== //
// デバイスの生成・初期化のみを行う //
// =========================== //

#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include <wrl.h>

class GraphicsDevice {
public:
	GraphicsDevice();
	~GraphicsDevice() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	// --- アクセッサ --- //
	ID3D12Device* GetDevice() const { return device_.Get(); }
	IDXGIFactory7* GetDxgiFactory() const { return dxgiFactory_.Get(); }
	IDxcUtils* GetDxcUtils() const { return dxcUtils_.Get(); }
	IDxcCompiler3* GetDxcCompiler() const { return dxcCompiler_.Get(); }
	IDxcIncludeHandler* GetIncludeHandler() const { return includeHandler_.Get(); }

public:
	// コピーと移動の禁止
	GraphicsDevice(const GraphicsDevice&) = delete;
	GraphicsDevice& operator=(const GraphicsDevice&) = delete;
	GraphicsDevice(GraphicsDevice&&) = delete;
	GraphicsDevice& operator=(GraphicsDevice&&) = delete;

private:
	void EnableDebugLayer();
	void SetupInfoQueue();

private:
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;

	// DXCシェーダーコンパイラ関連
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;
};