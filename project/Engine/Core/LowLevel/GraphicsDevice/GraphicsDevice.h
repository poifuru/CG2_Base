#pragma once

// ＝＝＝＝＝＝＝＝＝＝ //
// デバイスの保持を行う //
// ＝＝＝＝＝＝＝＝＝＝ //

class GraphicsDevice {
public:
	GraphicsDevice() = default;
	~GraphicsDevice() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// Deviceを取得
	/// </summary>
	/// <returns></returns>
	ID3D12Device* GetDevice() const { return device_.Get(); }

	/// <summary>
	/// DxgiFactoryを取得
	/// </summary>
	/// <returns></returns>
	IDXGIFactory7* GetDxgiFactory() const { return dxgiFactory_.Get(); }

public:
	// コピーと移動の禁止
	GraphicsDevice(const GraphicsDevice&) = delete;
	GraphicsDevice& operator=(const GraphicsDevice&) = delete;
	GraphicsDevice(GraphicsDevice&&) = delete;
	GraphicsDevice& operator=(GraphicsDevice&&) = delete;

private:
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;
};