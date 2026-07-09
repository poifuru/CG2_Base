#pragma once
#include <memory>
#include <string>

class Model;
// 前方宣言
namespace MyEngine::LowLevel {
	class GraphicsDevice;
}
class DescriptorHeapManager;
class ModelManager;
class TextureManager;
class ShaderManager;

class ModelFactory {
public:
	ModelFactory() = default;
	~ModelFactory() = default;

	void Initialize(
		MyEngine::LowLevel::GraphicsDevice* device,
		DescriptorHeapManager* heapManager,
		ModelManager* modelManager,
		TextureManager* textureManager,
		ShaderManager* shaderManager
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
	DescriptorHeapManager* heapManager_ = nullptr;
	ModelManager* modelManager_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	ShaderManager* shaderManager_ = nullptr;
};
