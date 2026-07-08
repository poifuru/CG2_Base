#pragma once

// ＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝ //
// デバイスの生成・デバッグレイヤーなどのセットアップを行う //
// ＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝ //

namespace MyEngine::LowLevel {
	class GraphicsDeviceCreator {
	public:
		static void CreateAndSetup(
			Microsoft::WRL::ComPtr<ID3D12Device>& device,
			Microsoft::WRL::ComPtr<IDXGIFactory7>& dxgiFactory
		);
	};
}