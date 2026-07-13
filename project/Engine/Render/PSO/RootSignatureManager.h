#pragma once

namespace MyEngine::Rendering {
	class RootSignatureManager {
	public:		// メンバ関数
		RootSignatureManager() = default;
		~RootSignatureManager() = default;

		void Initialize(ID3D12Device* device);

		// レンダラーやPSOManagerが使うゲッター
		ID3D12RootSignature* GetCommonRootSignature() const { return rootSignature_.Get(); }

	public:
		// コピー・移動禁止
		RootSignatureManager(const RootSignatureManager&) = delete;
		RootSignatureManager& operator=(const RootSignatureManager&) = delete;
		RootSignatureManager(RootSignatureManager&&) = delete;
		RootSignatureManager& operator=(RootSignatureManager&&) = delete;

	private:	// メンバ変数
		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	};
}