#pragma once
#include "GameObject.h"

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

	struct Mesh;
	class Material;

	enum class ShadingModel : uint8_t;
	enum class BlendModeType : uint8_t;
	enum class InputLayoutType : uint32_t;

	struct ShaderPair {
		uint32_t vs_ID;
		uint32_t ps_ID;
	};

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

		void Draw(std::vector<std::unique_ptr<GameObject>>& objects);

		MyEngine::Rendering::RenderSystem* GetRenderSystem() { return renderSystem_.get(); }
		MyEngine::Rendering::RenderTexture* GetRenderTexture() { return renderTexture_.get(); }
		MyEngine::Rendering::RootSignatureManager* GetRootSigManager() { return rootSigManager_.get(); }
		MyEngine::Rendering::PSOManager* GetPSOManager() { return psoManager_.get(); }
		MyEngine::Rendering::ShaderManager* GetShaderManager() { return shaderManager_.get(); }
		MyEngine::Rendering::InputLayoutManager* GetInputLayoutManager() { return inputLayoutManager_.get(); }
		MyEngine::Rendering::BlendModeManager* GetBlendModeManager() { return blendModeManager_.get(); }

	private:
		uint64_t MakeShaderKey(
			MyEngine::Rendering::ShadingModel model,
			MyEngine::Rendering::InputLayoutType layout
		);

		void InitializeShaderTable();

		void Submit(
			const MyEngine::Rendering::Mesh& mesh,
			MyEngine::Rendering::Material* material,
			D3D12_GPU_VIRTUAL_ADDRESS transformAddr,
			const char* debugName
		);

	private:
		ID3D12Device* device_ = nullptr;
		std::unique_ptr<MyEngine::Rendering::RootSignatureManager> rootSigManager_;
		std::unique_ptr<MyEngine::Rendering::ShaderManager> shaderManager_;
		std::unique_ptr<MyEngine::Rendering::InputLayoutManager> inputLayoutManager_;
		std::unique_ptr<MyEngine::Rendering::BlendModeManager> blendModeManager_;
		std::unique_ptr<MyEngine::Rendering::PSOManager> psoManager_;

		std::unique_ptr<MyEngine::Rendering::RenderSystem> renderSystem_;
		std::unique_ptr<MyEngine::Rendering::RenderTexture> renderTexture_;

		// シェーダーの組み合わせを保存しておく
		std::unordered_map<uint64_t, ShaderPair> shaderTable_;
	};
}