#include "PCH.h"
#include "Model.h"
#include "MeshData.h"

MyEngine::Rendering::Model::Model() : BaseObject3d() {
}

void MyEngine::Rendering::Model::Model::Initialize(MyEngine::Rendering::ModelData* modelData) {
	BaseObject3d::Initialize();

	modelData_ = modelData;
}

MyEngine::Rendering::Material * MyEngine::Rendering::Model::GetMaterial()
{
	return material_.get();
}

void MyEngine::Rendering::Model::Model::SetShaders(MyEngine::Rendering::ShadingModel shadingModel)  {
	if (material_) {
		material_->SetShadingModel(shadingModel);
	}
}

void MyEngine::Rendering::Model::Model::SetTextureIndex(uint32_t textureIndex)  {
	if (material_) {
		material_->SetTextureIndex(textureIndex);
	}
}