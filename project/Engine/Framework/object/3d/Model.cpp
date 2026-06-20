#include "Model.h"
#include "RenderSystem.h"

Model::Model() : BaseObject3d() {
}

void Model::Initialize(ModelData* modelData) {
	BaseObject3d::Initialize();

	modelData_ = modelData;
	layer_ = 0; // 不透明
}

void Model::Draw(RenderSystem* renderSystem) {
	if(!modelData_ || !renderSystem) return;

	for (const auto& mesh : modelData_->meshes) {
		// RenderCommandの組み立てと積み込み
		RenderCommand cmd{};

		cmd.psoDesc.VS_ID = vsID_;
		cmd.psoDesc.PS_ID = psID_;
		cmd.psoDesc.InputLayoutID = InputLayoutType::Standard3D;
		cmd.psoDesc.BlendMode = BlendModeType::Opaque;
		cmd.psoDesc.CullMode = D3D12_CULL_MODE_NONE; // 両面表示

		// メッシュ情報の設定
		cmd.vbView = mesh.vbView;
		cmd.ibv = mesh.ibView;
		cmd.indexCount = mesh.indexCount;

		// バインドレスマテリアルのインデックスとテクスチャインデックスを設定
		cmd.materialIndex = GetMaterialDescriptorIndex();
		cmd.textureIndex = (textureIndex_ != 0) ? textureIndex_ : mesh.textureIndex;
		
		// トランスフォームバッファのGPUアドレスを設定
		cmd.transformGPUAddress = GetTransformGPUAddress();
		
		cmd.layer = layer_;

		// コマンドを積む
		renderSystem->PushCommand(cmd);
	}
}