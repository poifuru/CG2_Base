#pragma once

class Model;
// 前方宣言
namespace MyEngine::LowLevel {
	class GraphicsDevice;
	class DescriptorHeapManager;
}

namespace MyEngine::Rendering {
	class RootSignatureManager;
	class ShaderManager;
	class InputLayoutManager;
	class BlendModeManager;
	class PSOManager;
}

class ModelManager;
class TextureManager;

class ModelFactory {
public:
	ModelFactory() = default;
	~ModelFactory() = default;

	void Initialize(
		MyEngine::LowLevel::GraphicsDevice* device,
		MyEngine::LowLevel::DescriptorHeapManager* heapManager,
		MyEngine::Rendering::RootSignatureManager* rootSigManager, 
		MyEngine::Rendering::PSOManager* psoManager,
		MyEngine::Rendering::ShaderManager* shaderManager,
		MyEngine::Rendering::InputLayoutManager* inputLayoutManager,
		MyEngine::Rendering::BlendModeManager* blendModeManager,
		ModelManager* modelManager,
		TextureManager* textureManager
	);

	// アセットが紐づいたModelを組み立てて出力する
	std::unique_ptr<Model> CreateModel(
		uint32_t modelIndex,
		uint32_t textureIndex = 0
	);

public:
	// コピー・移動禁止
	ModelFactory(const ModelFactory&) = delete;
	ModelFactory& operator=(const ModelFactory&) = delete;
	ModelFactory(ModelFactory&&) = delete;
	ModelFactory& operator=(ModelFactory&&) = delete;

private:
	MyEngine::LowLevel::GraphicsDevice* device_ = nullptr;
	MyEngine::LowLevel::DescriptorHeapManager* heapManager_ = nullptr;
	MyEngine::Rendering::RootSignatureManager* rootSigManager_ = nullptr;
	MyEngine::Rendering::PSOManager* psoManager_ = nullptr;
	MyEngine::Rendering::ShaderManager* shaderManager_ = nullptr;
	MyEngine::Rendering::InputLayoutManager* inputLayoutManager_ = nullptr;
	MyEngine::Rendering::BlendModeManager* blendModeManager_ = nullptr;

	ModelManager* modelManager_ = nullptr;
	TextureManager* textureManager_ = nullptr;
};
