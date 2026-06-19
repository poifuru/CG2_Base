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
	triangleData->vertexCount = 3;
	triangleData->indexCount = 3;

	// 頂点データ
	VertexData v0{ { 0.0f,  0.5f, 0.0f, 1.0f }, { 0.5f, 0.0f }, { 0.0f, 0.0f, -1.0f } };
	VertexData v1{ { 0.5f, -0.5f, 0.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } };
	VertexData v2{ {-0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } };
	triangleData->meshData.vertices = { v0, v1, v2 };
	triangleData->meshData.indices = { 0, 1, 2 };

	// 頂点バッファ・インデックスバッファの生成と設定 (カプセル化されたInitializeを使用)
	triangleData->meshResource.Initialize(device_, triangleData->meshData);

	// ビューの情報を設定
	triangleData->vbView = triangleData->meshResource.vertexBuffer.GetView();
	triangleData->ibView = triangleData->meshResource.indexBuffer.GetView();

	// マップへ登録
	modelMap_["Triangle"] = triangleData;
}

ModelData* ModelManager::LoadModelData(const std::string& directoryPath, const std::string& fileName, bool inversion) {
	// すでに読み込まれていたら既存データを返す
	if(modelMap_.count(fileName)) {
		return modelMap_.at(fileName).get();
	}

	// 新規読み込み
	ModelData cpuData = LoadModelFile(directoryPath, fileName, inversion);
	std::shared_ptr<ModelData> newData = std::make_shared<ModelData>(std::move(cpuData));

	// 頂点バッファ・インデックスバッファの生成と設定 (カプセル化されたInitializeを使用)
	newData->meshResource.Initialize(device_, newData->meshData);

	// ビューの情報を設定
	newData->vbView = newData->meshResource.vertexBuffer.GetView();
	newData->ibView = newData->meshResource.indexBuffer.GetView();

	modelMap_[fileName] = newData;

	return modelMap_.at(fileName).get();
}

std::weak_ptr<ModelData> ModelManager::GetModelData(std::string id) {
	assert(modelMap_.count(id));
	return modelMap_.at(id);
}

void ModelManager::UnloadModelData(const std::string& id) {
	modelMap_.erase(id);
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

ModelData ModelManager::LoadModelFile(const std::string& directoryPath, const std::string& fileName, bool inversion) {
	ModelData modelData;
	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + fileName;
	const aiScene* scene = importer.ReadFile(
		filePath.c_str(),
		aiProcess_FlipWindingOrder |
		aiProcess_FlipUVs |
		aiProcess_JoinIdenticalVertices
	);
	assert(scene->HasMeshes());

	for(uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());
		assert(mesh->HasTextureCoords(0));

		for(uint32_t i = 0; i < mesh->mNumVertices; ++i) {
			aiVector3D& position = mesh->mVertices[i];
			aiVector3D& normal = mesh->mNormals[i];
			aiVector3D& texcoord = mesh->mTextureCoords[0][i];

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
			modelData.meshData.vertices.push_back(vertex);
		}

		for(uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);

			for(uint32_t i = 0; i < face.mNumIndices; ++i) {
				modelData.meshData.indices.push_back(face.mIndices[i]);
			}
		}

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

	for(uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		if(material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			modelData.textureFilePath = directoryPath + "/" + textureFilePath.C_Str();
		}
	}

	modelData.vertexCount = static_cast<uint32_t>(modelData.meshData.vertices.size());
	modelData.indexCount = static_cast<uint32_t>(modelData.meshData.indices.size());
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