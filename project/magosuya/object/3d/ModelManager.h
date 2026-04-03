#pragma once
#include <Windows.h>
#include <wrl.h>
using namespace Microsoft::WRL;
#include <unordered_map>
#include <string>
#include <queue>
#include <vector>
#include <optional>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "struct.h"

class DxCommon;
class TextureManager;

class ModelManager {
public:		//メンバ関数
	static ModelManager* GetInstance () {
		//初めて呼び出されたときに一回だけ初期化
		static ModelManager instance;
		return &instance;
	}

	void Initialize (DxCommon* dxCommon, TextureManager* textureManager);

	ModelData* LoadModelData (const std::string& directoryPath, const std::string& fileName, bool inversion = false);
	std::weak_ptr<ModelData> GetModelData (std::string id);
	void UnloadModelData (const std::string& id);

	Animation* LoadAnimationData(const std::string& directoryPath, const std::string& fileName);
	std::weak_ptr<Animation> GetAnimationData(std::string id);
	void UnloadAnimationData(const std::string& id);

	

private:
	//コンストラクタを禁止
	ModelManager () = default;
	// コピーコンストラクタと代入演算子を禁止
	ModelManager (const ModelManager&) = delete;
	ModelManager& operator=(const ModelManager&) = delete;
	ModelManager (ModelManager&&) = delete;
	ModelManager& operator=(ModelManager&&) = delete;

private:	//内部関数
	//マテリアルファイルの読み込み関数
	MaterialFile LoadMaterialTemplateFile (const std::string& directoryPath, const std::string& id);

	//モデル読み込みの関数
	ModelData LoadModelFile (const std::string& directoryPath, const std::string& fileName, bool inversion);

	//assimpのノードからNode構造体に変換する関数
	Node ReadNode(aiNode* node);

	//アニメーション読み込み関数
	Animation LoadAnimation(const std::string& directoryPath, const std::string& fileName);

private:	//メンバ変数
	std::unordered_map<std::string, std::shared_ptr<ModelData>> modelMap_;
	std::unordered_map<std::string, std::shared_ptr<Animation>> animationMap_;

	//ポインタを借りる
	DxCommon* dxCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;
};