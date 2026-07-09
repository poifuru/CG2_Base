#pragma once

// 前方宣言
namespace MyEngine::LowLevel {
	class DescriptorHeapManager;
}
class RootSignatureManager;
class ShaderManager;
class InputLayoutManager;
class BlendModeManager;
class PSOManager;
class RenderSystem;
class RenderTexture;

namespace MyEngine::Rendering {
	class Renderer {
	public:
		void Initialize(
		ID3D12Device* device,
		IDxcUtils* dxcUtils,
		IDxcCompiler3* dxcCompiler,
		IDxcIncludeHandler* includeHandler,
		MyEngine::LowLevel::DescriptorHeapManager* heapManager
		);

		RenderSystem* GetRenderSystem() { return renderSystem_.get(); }
		ShaderManager* GetShaderManager() { return shaderManager_.get(); }
		RenderTexture* GetRenderTexture() { return renderTexture_.get(); }

	private:
		std::unique_ptr<RootSignatureManager> rootSigManager_;
		std::unique_ptr<ShaderManager> shaderManager_;
		std::unique_ptr<InputLayoutManager> inputLayoutManager_;
		std::unique_ptr<BlendModeManager> blendModeManager_;
		std::unique_ptr<PSOManager> psoManager_;

		std::unique_ptr<RenderSystem> renderSystem_;
		std::unique_ptr<RenderTexture> renderTexture_;
	};
}