#include "PCH.h"
#include "Model.h"
#include "Renderer.h"
#include "RenderSystem.h"
#include "RenderCommand.h"
#include "InputLayoutManager.h"

Model::Model() : BaseObject3d() {
}

void Model::Initialize(ModelData* modelData) {
	BaseObject3d::Initialize();

	modelData_ = modelData;
	layer_ = 0; // 不透明
}

void Model::Draw(MyEngine::Rendering::Renderer* renderer) {
	if(!modelData_ || !renderer) return;

	for (size_t i = 0; i < modelData_->meshes.size(); ++i) {
		auto& mesh = modelData_->meshes[i];

		// ★抽象化された窓口(Submit)に、必要なデータとマテリアルを渡す
		renderer->Submit(
			mesh.vbView,
			mesh.ibView,
			mesh.indexCount,
			GetTransformGPUAddress(),
			material_.get(),
			layer_
		);
	}
}