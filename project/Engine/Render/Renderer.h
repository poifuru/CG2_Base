#pragma once

// 前方宣言
namespace MyEngine::LowLevel {
	class DescriptorHeapManager;
}

namespace MyEngine::Rendering {
	class RenderTexture;
	class RenderSystem;
	class RootSignatureManager;
	class ShaderManager;
	class InputLayoutManager;
	class BlendModeManager;
	class PSOManager;
}

class Material;

namespace MyEngine::Rendering {
	class Renderer {
	public:
		Renderer();
		~Renderer();

		void Initialize(
			ID3D12Device* device,
			IDxcUtils* dxcUtils,
			IDxcCompiler3* dxcCompiler,
			IDxcIncludeHandler* includeHandler,
			MyEngine::LowLevel::DescriptorHeapManager* heapManager
		);

		void RenderScene(ID3D12GraphicsCommandList* cmdList);

		void Submit(
			const D3D12_VERTEX_BUFFER_VIEW& vbView,
			const D3D12_INDEX_BUFFER_VIEW& ibv,
			uint32_t indexCount,
			D3D12_GPU_VIRTUAL_ADDRESS transformGPUAddress,
			Material* material,
			uint32_t layer
		);

		MyEngine::Rendering::RenderSystem* GetRenderSystem() { return renderSystem_.get(); }
		MyEngine::Rendering::RenderTexture* GetRenderTexture() { return renderTexture_.get(); }
		MyEngine::Rendering::RootSignatureManager* GetRootSigManager() { return rootSigManager_.get(); }
		MyEngine::Rendering::PSOManager* GetPSOManager() { return psoManager_.get(); }
		MyEngine::Rendering::ShaderManager* GetShaderManager() { return shaderManager_.get(); }
		MyEngine::Rendering::InputLayoutManager* GetInputLayoutManager() { return inputLayoutManager_.get(); }
		MyEngine::Rendering::BlendModeManager* GetBlendModeManager() { return blendModeManager_.get(); }

	private:
		std::unique_ptr<MyEngine::Rendering::RootSignatureManager> rootSigManager_;
		std::unique_ptr<MyEngine::Rendering::ShaderManager> shaderManager_;
		std::unique_ptr<MyEngine::Rendering::InputLayoutManager> inputLayoutManager_;
		std::unique_ptr<MyEngine::Rendering::BlendModeManager> blendModeManager_;
		std::unique_ptr<MyEngine::Rendering::PSOManager> psoManager_;

		std::unique_ptr<MyEngine::Rendering::RenderSystem> renderSystem_;
		std::unique_ptr<MyEngine::Rendering::RenderTexture> renderTexture_;
	};
}