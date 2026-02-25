#include "ModelManager.h"
#include <sstream>
#include <cassert>
#include <map>
#include <filesystem>
#include "MagosuyaEngine.h"
#include "DxCommon.h"
#include "TextureManager.h"

namespace fs = std::filesystem;

void ModelManager::Initialize(DxCommon* dxCommon, TextureManager* textureManager) {
	dxCommon_ = dxCommon;
	textureManager_ = textureManager;
}

ModelData* ModelManager::LoadModelData(const std::string& directoryPath, const std::string& fileName, bool inversion) {
	//IDのモデルをすでに読み込んでいたら
	if(modelMap_.count(fileName)) {
		//既存データを取得
		std::shared_ptr<ModelData> existingData = modelMap_.at(fileName);
		//存在していたら既存のデータを返す
		return existingData.get();
	}

	//読み込んでいなければ新規読み込み
	ModelData cpuData = LoadModelFile(directoryPath, fileName, inversion);
	//shared_ptrに入れてGPUリソース作成の準備
	std::shared_ptr<ModelData> newData = std::make_shared<ModelData>(std::move(cpuData));

	//頂点バッファの生成と設定
	newData->vertexBuffer = dxCommon_->CreateBufferResource(sizeof(VertexData) * newData->vertexCount);
	//頂点バッファにデータを書き込む
	VertexData* vertexDataPtr = nullptr;
	newData->vertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataPtr));
	//CPUメモリからGPUリソースへデータをコピー
	memcpy(vertexDataPtr, newData->vertices.data(), sizeof(VertexData) * newData->vertexCount);
	newData->vertexBuffer->Unmap(0, nullptr);

	//頂点バッファビューの設定
	newData->vbView.BufferLocation = newData->vertexBuffer->GetGPUVirtualAddress();
	newData->vbView.SizeInBytes = UINT(sizeof(VertexData) * newData->vertexCount);
	newData->vbView.StrideInBytes = sizeof(VertexData);

	//インデックスバッファの生成と設定
	newData->indexBuffer = dxCommon_->CreateBufferResource(sizeof(uint32_t) * newData->indexCount);

	//インデックスバッファにデータを書き込む
	uint32_t* indexDataPtr = nullptr;
	newData->indexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&indexDataPtr));
	//CPUメモリからGPUリソースへデータをコピー
	memcpy(indexDataPtr, newData->indices.data(), sizeof(uint32_t) * newData->indexCount);
	newData->indexBuffer->Unmap(0, nullptr);

	//インデックスバッファビューの設定
	newData->ibView.BufferLocation = newData->indexBuffer->GetGPUVirtualAddress();
	newData->ibView.SizeInBytes = UINT(sizeof(uint32_t) * newData->indexCount);
	newData->ibView.Format = DXGI_FORMAT_R32_UINT;

	//mapに登録
	modelMap_[fileName] = newData;

	// CPUメモリを解放（必要に応じて）
	newData->vertices.clear();
	newData->indices.clear();

	//データ提供
	return modelMap_.at(fileName).get();
}

std::weak_ptr<ModelData> ModelManager::GetModelData(std::string id) {
	//ID指定してmapから持ってくる
	assert(modelMap_.count(id));
	return modelMap_.at(id);
}

void ModelManager::UnloadModelData(const std::string& id) {
	//キャッシュマップからデータを削除
	modelMap_.erase(id);
}

Animation* ModelManager::LoadAnimationData(const std::string& directoryPath, const std::string& fileName) {
	//IDのAnimationをすでに読み込んでいたら
	if(animationMap_.count(fileName)) {
		//既存データを取得
		std::shared_ptr<Animation> existingData = animationMap_.at(fileName);
		//存在していたら既存のデータを返す
		return existingData.get();
	}

	// 読み込んでいなければ新規読み込み
	Animation loadData = LoadAnimation(directoryPath, fileName);
	// shared_ptrに入れる
	std::shared_ptr<Animation> newData = std::make_shared<Animation>(std::move(loadData));

	// newDataをmapに登録
	animationMap_[fileName] = newData;

	// データ提供
	return animationMap_.at(fileName).get();
}

std::weak_ptr<Animation> ModelManager::GetAnimationData(std::string id) {
	//ID指定してmapから持ってくる
	assert(animationMap_.count(id));
	return animationMap_.at(id);
}

void ModelManager::UnloadAnimationData(const std::string& id) {
	//キャッシュマップからデータを削除
	animationMap_.erase(id);
}

MaterialFile ModelManager::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& id) {
	//必要な変数の宣言
	MaterialFile materialData;
	std::string line;

	//ファイルを開く
	std::ifstream file(directoryPath + "/" + id);
	assert(file.is_open());

	//実際にファイルを読み、MaterialDataを構築していく
	while(std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		//identifierに応じた処理
		if(identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			//連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}

	//MaterialDataを返す
	return materialData;
}

ModelData ModelManager::LoadModelFile(const std::string& directoryPath, const std::string& fileName, bool inversion) {
	//必要になる変数宣言
	ModelData modelData;	//構築するModelData

	//assimpで読み込み
	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + fileName;
	const aiScene* scene = importer.ReadFile(
		filePath.c_str(),
		aiProcess_FlipWindingOrder |
		aiProcess_FlipUVs |
		aiProcess_JoinIdenticalVertices		//インデックス描画対応用
	);
	assert(scene->HasMeshes());	//メッシュが無ければ対応しない

	//Meshを解析する
	for(uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());
		assert(mesh->HasTextureCoords(0));

		//vertexの解析
		for(uint32_t i = 0; i < mesh->mNumVertices; ++i) {
			aiVector3D& position = mesh->mVertices[i];
			aiVector3D& normal = mesh->mNormals[i];
			aiVector3D& texcoord = mesh->mTextureCoords[0][i];

			VertexData vertex;
			//aiProcess_MakeLeftHandedはz*-1	で、右手->左手に変換するので手動で対処
			if(inversion) {
				vertex.position = { position.x, position.y, position.z, 1.0f };
				vertex.normal = { normal.x, normal.y, normal.z };
			}
			else {
				vertex.position = { -position.x, position.y, position.z, 1.0f };
				vertex.normal = { -normal.x, normal.y, normal.z };
			}
			vertex.texcoord = { texcoord.x, texcoord.y };
			modelData.vertices.push_back(vertex);
		}

		//Faceを解析する
		for(uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);	//三角形のみサポート

			//indexの解析
			for(uint32_t i = 0; i < face.mNumIndices; ++i) {
				modelData.indices.push_back(face.mIndices[i]);
			}
		}
	}

	//materialの解析
	for(uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		if(material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			modelData.material.textureFilePath = directoryPath + "/" + textureFilePath.C_Str();
		}
	}

	// 頂点数を取得 (重複が排除されたユニークな数になっているはず)
	modelData.vertexCount = static_cast<uint32_t>(modelData.vertices.size());

	// インデックス数を取得
	modelData.indexCount = static_cast<uint32_t>(modelData.indices.size());

	//RootNodeを読んで階層構造を作り上げる
	modelData.rootNode = ReadNode(scene->mRootNode);

	// ModelDataを返す
	return modelData;
}

Node ModelManager::ReadNode(aiNode* node) {
	Node result;
	
	aiVector3D scale;
	aiQuaternion rotate;
	aiVector3D translate;

	aiMatrix4x4 aiLocalMatrix = node->mTransformation; //nodeのlocalMatrixを取得

	for(int i = 0; i < 4; ++i) {
		for(int j = 0; j < 4; ++j) {
			result.localMatrix.m[i][j] = aiLocalMatrix[i][j];
		}
	}
	result.name = node->mName.C_Str();	//Node名を格納
	result.children.resize(node->mNumChildren);	//子供の数だけサイズを確保
	for(uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		//再帰的に読んで階層構造を作っていく
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}

	return result;
}

Animation ModelManager::LoadAnimation(const std::string& directoryPath, const std::string& fileName) {
	Animation animation = {};	// 今回作るアニメーション
	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + fileName;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), 0);
	// アニメーションがなかったらassert
	assert(scene->mNumAnimations != 0);
	// ひとまず最初のアニメーションだけ採用(後から複数対応可)
	aiAnimation* animationAssimp = scene->mAnimations[0];
	// Animationの長さを秒単位に変換する
	// mTickPerSecond : 周波数, mDuration : mTickPerSecondで指定された周波数に置ける長さ
	animation.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond);

	//NodeAnimationの解析
	//assimpでは個々のnodeのanimationをchannelと呼んでいるのでchannelを回して情報をとる
	// translate
	for(uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex) {
		aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
		NodeAnimation& nodeAnimation = animation.nodeAnimaitons[nodeAnimationAssimp->mNodeName.C_Str()];
		for(uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex) {
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
			KeyframeVector3 keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);	// 秒単位に変換
			keyframe.value = { -keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z };	// 右手->左手
			nodeAnimation.translate.keyframes.push_back(keyframe);
		}
		// rotate
		for(uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
			aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
			KeyframeQuaternion keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);	// 秒単位に変換
			keyframe.value = { -keyAssimp.mValue.x, -keyAssimp.mValue.y, keyAssimp.mValue.z, keyAssimp.mValue.w };	// 右手->左手
			nodeAnimation.rotate.keyframes.push_back(keyframe);
		}
		// scale
		for(uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex) {
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
			KeyframeVector3 keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);	// 秒単位に変換
			keyframe.value = { keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z };
			nodeAnimation.scale.keyframes.push_back(keyframe);
		}
	}
	return animation;
}
