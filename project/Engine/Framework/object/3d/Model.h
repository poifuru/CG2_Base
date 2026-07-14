#pragma once
#include "BaseObject3d.h"

namespace MyEngine::Rendering {
	struct ModelData;
}

namespace MyEngine::Rendering {
	class Model : public BaseObject3d {
	public:
		Model();
		~Model() override = default;

		// アセットはファクトリーから直接流してもらう
		void Initialize(MyEngine::Rendering::ModelData* modelData);

		MyEngine::Rendering::Material* GetMaterial();

		// モデルデータを外に渡せるように
		MyEngine::Rendering::ModelData* GetModelData() const { return modelData_; }
		void SetModelData(MyEngine::Rendering::ModelData* modelData) { modelData_ = modelData; }

		// シェーディングモデルの設定
		void SetShaders(MyEngine::Rendering::ShadingModel shadingModel);
		void SetTextureIndex(uint32_t textureIndex);

	private:
		MyEngine::Rendering::ModelData* modelData_ = nullptr;
	};
}