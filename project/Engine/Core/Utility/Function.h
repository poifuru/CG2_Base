#pragma once

namespace MyEngine::Utility {
	//クラッシュハンドルを登録するための関数
	LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);

	//DepthStencilTexture作成関数
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height);

	/// <summary>
	/// バリアの変更
	/// </summary>
	/// <param name="cmdList"></param>
	/// <param name="resource"></param>
	/// <param name="stateBefore"></param>
	/// <param name="stateAfter"></param>
	void TransitionBarrier(
		ID3D12GraphicsCommandList* cmdList,
		ID3D12Resource* resource,
		D3D12_RESOURCE_STATES stateBefore,
		D3D12_RESOURCE_STATES stateAfter
	);
}