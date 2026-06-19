#pragma once
#include <memory>
#include <string>

class Model;
class GraphicsDevice;
class DescriptorHeapManager;
class ModelManager;
class TextureManager;

class ModelFactory {
public:
	ModelFactory() = default;
	ModelFactory(
		GraphicsDevice* device,
		DescriptorHeapManager* heapManager,
		ModelManager* modelManager,
		TextureManager* textureManager
	);
	~ModelFactory() = default;

	void Initialize(
		GraphicsDevice* device,
		DescriptorHeapManager* heapManager,
		ModelManager* modelManager,
		TextureManager* textureManager
	);

	// アセットが紐づいたModelを組み立てて出力する
	std::unique_ptr<Model> CreateModel(
		const std::string& modelName,
		const std::string& textureName
	);

public:
	// コピー・移動禁止
	ModelFactory(const ModelFactory&) = delete;
	ModelFactory& operator=(const ModelFactory&) = delete;
	ModelFactory(ModelFactory&&) = delete;
	ModelFactory& operator=(ModelFactory&&) = delete;

private:
	GraphicsDevice* device_ = nullptr;
	DescriptorHeapManager* heapManager_ = nullptr;
	ModelManager* modelManager_ = nullptr;
	TextureManager* textureManager_ = nullptr;
};
