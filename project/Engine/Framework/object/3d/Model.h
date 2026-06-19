#pragma once
#include "BaseObject3d.h"
#include "MeshData.h"

class RenderSystem;

class Model : public BaseObject3d {
public:
	Model();
	~Model() override = default;

	// アセットはファクトリーから直接流してもらう
	void Initialize(ModelData* modelData);

	void Draw(RenderSystem* renderSystem) override;

	// モデルデータを外に渡せるように
	ModelData* GetModelData() const { return modelData_; }
	void SetModelData(ModelData* modelData) { modelData_ = modelData; }

	// シェーダーIDやテクスチャインデックスの設定
	void SetShaders(uint32_t vsID, uint32_t psID) {
		vsID_ = vsID;
		psID_ = psID;
	}
	void SetTextureIndex(uint32_t textureIndex) {
		textureIndex_ = textureIndex;
	}

private:
	ModelData* modelData_ = nullptr;

	uint32_t vsID_ = 0;
	uint32_t psID_ = 0;
	uint32_t textureIndex_ = 0;
};