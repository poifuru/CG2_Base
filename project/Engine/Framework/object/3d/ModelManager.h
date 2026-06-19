#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "struct.h"
#include "MeshData.h"
#include "Animation.h"

class TextureManager;
struct ID3D12Device;

class ModelManager {
public:
	ModelManager() = default;
	~ModelManager() = default;

	void Initialize(ID3D12Device* device, TextureManager* textureManager);

	ModelData* LoadModelData(const std::string& directoryPath, const std::string& fileName, bool inversion = false);
	std::weak_ptr<ModelData> GetModelData(std::string id);
	void UnloadModelData(const std::string& id);

	Animation* LoadAnimationData(const std::string& directoryPath, const std::string& fileName);
	std::weak_ptr<Animation> GetAnimationData(std::string id);
	void UnloadAnimationData(const std::string& id);

public:
	// コピー・移動禁止
	ModelManager(const ModelManager&) = delete;
	ModelManager& operator=(const ModelManager&) = delete;
	ModelManager(ModelManager&&) = delete;
	ModelManager& operator=(ModelManager&&) = delete;

private:
	// マテリアルファイルの読み込み関数
	MaterialFile LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& id);

	// モデル読み込みの関数
	ModelData LoadModelFile(const std::string& directoryPath, const std::string& fileName, bool inversion);

	// assimpのノードからNode構造体に変換する関数
	Node ReadNode(aiNode* node);

	// アニメーション読み込み関数
	Animation LoadAnimation(const std::string& directoryPath, const std::string& fileName);

	// DX12バッファ生成用の内部ヘルパー
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

private:
	std::unordered_map<std::string, std::shared_ptr<ModelData>> modelMap_;
	std::unordered_map<std::string, std::shared_ptr<Animation>> animationMap_;

	ID3D12Device* device_ = nullptr;
	TextureManager* textureManager_ = nullptr;
};