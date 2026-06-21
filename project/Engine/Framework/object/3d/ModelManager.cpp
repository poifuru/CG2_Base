#include "ModelManager.h"
#include <sstream>
#include <fstream>
#include <cassert>
#include <map>
#include <filesystem>
#include "TextureManager.h"
#include "MathFunction.h"

namespace fs = std::filesystem;

void ModelManager::Initialize(ID3D12Device* device, TextureManager* textureManager) {
	device_ = device;
	textureManager_ = textureManager;

	// --- デフォルトの三角形アセット "Triangle" のダミー作成 ---
	auto triangleData = std::make_shared<ModelData>();
	
	Mesh triangleMesh{};
	triangleMesh.vertexCount = 3;
	triangleMesh.indexCount = 3;

	// 頂点データ
	VertexData v0{ { 0.0f,  0.5f, 0.0f, 1.0f }, { 0.5f, 0.0f }, { 0.0f, 0.0f, -1.0f } };
	VertexData v1{ { 0.5f, -0.5f, 0.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } };
	VertexData v2{ {-0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } };
	triangleMesh.meshData.vertices = { v0, v1, v2 };
	triangleMesh.meshData.indices = { 0, 1, 2 };

	// 頂点バッファ・インデックスバッファの生成と設定
	triangleMesh.meshResource.Initialize(device_, triangleMesh.meshData);

	// ビューの情報を設定
	triangleMesh.vbView = triangleMesh.meshResource.vertexBuffer.GetView();
	triangleMesh.ibView = triangleMesh.meshResource.indexBuffer.GetView();

	triangleData->meshes.push_back(std::move(triangleMesh));

	// ベクターへ登録
	models_.push_back(triangleData);
	modelPathToIndexMap_["Triangle"] = 0;
}

uint32_t ModelManager::LoadModelData(const std::string& filePath, bool inversion) {
	// すでに読み込まれていたら既存データを返す
	if(modelPathToIndexMap_.count(filePath)) {
		return modelPathToIndexMap_.at(filePath);
	}

	// 新規読み込み
	ModelData cpuData = LoadModelFile(filePath, inversion);
	
	// 頂点バッファ・インデックスバッファの生成と設定、及びテクスチャ自動ロード
	for (auto& mesh : cpuData.meshes) {
		mesh.meshResource.Initialize(device_, mesh.meshData);
		mesh.vbView = mesh.meshResource.vertexBuffer.GetView();
		mesh.ibView = mesh.meshResource.indexBuffer.GetView();

		if (!mesh.textureFilePath.empty()) {
			mesh.textureIndex = textureManager_->LoadTexture(mesh.textureFilePath);
		} else {
			mesh.textureIndex = textureManager_->LoadTexture("white1x1");
		}
	}

	std::shared_ptr<ModelData> newData = std::make_shared<ModelData>(std::move(cpuData));

	uint32_t index = static_cast<uint32_t>(models_.size());
	models_.push_back(newData);
	modelPathToIndexMap_[filePath] = index;

	return index;
}

std::weak_ptr<ModelData> ModelManager::GetModelData(uint32_t index) {
	assert(index < models_.size());
	return models_[index];
}

void ModelManager::UnloadModelData(uint32_t index) {
	if (index < models_.size()) {
		models_[index].reset();
	}
}

Animation* ModelManager::LoadAnimationData(const std::string& directoryPath, const std::string& fileName) {
	if(animationMap_.count(fileName)) {
		return animationMap_.at(fileName).get();
	}

	Animation loadData = LoadAnimation(directoryPath, fileName);
	std::shared_ptr<Animation> newData = std::make_shared<Animation>(std::move(loadData));
	animationMap_[fileName] = newData;

	return animationMap_.at(fileName).get();
}

std::weak_ptr<Animation> ModelManager::GetAnimationData(std::string id) {
	assert(animationMap_.count(id));
	return animationMap_.at(id);
}

void ModelManager::UnloadAnimationData(const std::string& id) {
	animationMap_.erase(id);
}

MaterialFile ModelManager::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& id) {
	MaterialFile materialData;
	std::string line;

	std::ifstream file(directoryPath + "/" + id);
	assert(file.is_open());

	while(std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if(identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}

	return materialData;
}

ModelData ModelManager::LoadModelFile(const std::string& filePath, bool inversion) {
	ModelData modelData;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(
		filePath.c_str(),
		aiProcess_FlipWindingOrder |
		aiProcess_FlipUVs |
		aiProcess_JoinIdenticalVertices |
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals
	);
	assert(scene && scene->HasMeshes());

	// ファイルパスからディレクトリパスを抽出
	std::string directoryPath = std::filesystem::path(filePath).parent_path().string();

	for(uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];

		Mesh myMesh;

		for(uint32_t i = 0; i < mesh->mNumVertices; ++i) {
			aiVector3D& position = mesh->mVertices[i];
			aiVector3D normal = mesh->HasNormals() ? mesh->mNormals[i] : aiVector3D(0.0f, 1.0f, 0.0f);
			aiVector3D texcoord = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i] : aiVector3D(0.0f, 0.0f, 0.0f);

			VertexData vertex = {};
			if(inversion) {
				vertex.position = { position.x, position.y, position.z, 1.0f };
				vertex.normal = { normal.x, normal.y, normal.z };
			}
			else {
				vertex.position = { -position.x, position.y, position.z, 1.0f };
				vertex.normal = { -normal.x, normal.y, normal.z };
			}
			vertex.texcoord = { texcoord.x, texcoord.y };
			myMesh.meshData.vertices.push_back(vertex);
		}

		for(uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			if (face.mNumIndices != 3) {
				continue;
			}

			for(uint32_t i = 0; i < face.mNumIndices; ++i) {
				myMesh.meshData.indices.push_back(face.mIndices[i]);
			}
		}

		myMesh.vertexCount = static_cast<uint32_t>(myMesh.meshData.vertices.size());
		myMesh.indexCount = static_cast<uint32_t>(myMesh.meshData.indices.size());

		if (myMesh.vertexCount == 0 || myMesh.indexCount == 0) {
			continue;
		}

		// このメッシュに適用されているマテリアルのテクスチャパスを取得
		if(scene->mNumMaterials > 0 && mesh->mMaterialIndex < scene->mNumMaterials) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			if(material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
				aiString textureFilePath;
				material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
				myMesh.textureFilePath = directoryPath + "/" + textureFilePath.C_Str();
			}
		}

		modelData.meshes.push_back(std::move(myMesh));
	}

	for(uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		for(uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
			aiBone* bone = mesh->mBones[boneIndex];
			std::string jointName = bone->mName.C_Str();
			JointWeightData& jointWeightData = modelData.skinClusterData[jointName];

			const aiMatrix4x4& ai = bone->mOffsetMatrix;
			Matrix4x4 ibp;
			ibp.m[0][0] = ai.a1; ibp.m[0][1] = -ai.b1; ibp.m[0][2] = -ai.c1; ibp.m[0][3] = -ai.d1;
			ibp.m[1][0] = -ai.a2; ibp.m[1][1] = ai.b2; ibp.m[1][2] = ai.c2; ibp.m[1][3] = ai.d2;
			ibp.m[2][0] = -ai.a3; ibp.m[2][1] = ai.b3; ibp.m[2][2] = ai.c3; ibp.m[2][3] = ai.d3;
			ibp.m[3][0] = -ai.a4; ibp.m[3][1] = ai.b4; ibp.m[3][2] = ai.c4; ibp.m[3][3] = ai.d4;
			jointWeightData.inverseBindPoseMatrix = ibp;

			for(uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
				jointWeightData.vertexWeights.push_back(
					{ bone->mWeights[weightIndex].mWeight, bone->mWeights[weightIndex].mVertexId }
				);
			}
		}	
	}

	modelData.rootNode = ReadNode(scene->mRootNode);

	return modelData;
}

Node ModelManager::ReadNode(aiNode* node) {
	Node result;
	aiVector3D scale;
	aiQuaternion rotate;
	aiVector3D translate;
	node->mTransformation.Decompose(scale, rotate, translate);
	result.transform.scale = { scale.x, scale.y, scale.z };
	result.transform.rotate = { rotate.x, -rotate.y, -rotate.z, rotate.w };
	result.transform.translate = { -translate.x, translate.y, translate.z };
	result.localMatrix = Math::MakeAffineMatrix(
		result.transform.scale, result.transform.rotate, result.transform.translate
	);

	result.name = node->mName.C_Str();
	result.children.resize(node->mNumChildren);
	for(uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}

	return result;
}

Animation ModelManager::LoadAnimation(const std::string& directoryPath, const std::string& fileName) {
	Animation animation = {};
	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + fileName;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), 0);
	assert(scene->mNumAnimations != 0);
	aiAnimation* animationAssimp = scene->mAnimations[0];
	animation.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond);

	for(uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex) {
		aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
		NodeAnimation& nodeAnimation = animation.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];
		for(uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex) {
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
			KeyframeVector3 keyframe = {};
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = { -keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z };
			nodeAnimation.translate.keyframes.push_back(keyframe);
		}
		for(uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
			aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
			KeyframeQuaternion keyframe = {};
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = { keyAssimp.mValue.x, -keyAssimp.mValue.y, -keyAssimp.mValue.z, keyAssimp.mValue.w };
			nodeAnimation.rotate.keyframes.push_back(keyframe);
		}
		for(uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex) {
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
			KeyframeVector3 keyframe = {};
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = { keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z };
			nodeAnimation.scale.keyframes.push_back(keyframe);
		}
	}
	return animation;
}

Microsoft::WRL::ComPtr<ID3D12Resource> ModelManager::CreateBufferResource(size_t sizeInBytes) {
	D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	HRESULT hr = device_->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&resource)
	);
	assert(SUCCEEDED(hr));
	return resource;
}