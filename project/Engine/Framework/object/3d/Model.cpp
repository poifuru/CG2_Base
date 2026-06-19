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

	// RenderCommandの組み立てと積み込み
	RenderCommand cmd{};

	cmd.psoDesc.VS_ID = vsID_;
	cmd.psoDesc.PS_ID = psID_;
	cmd.psoDesc.InputLayoutID = InputLayoutType::Standard3D;
	cmd.psoDesc.BlendMode = BlendModeType::Opaque;
	cmd.psoDesc.CullMode = D3D12_CULL_MODE_NONE; // 両面表示
	cmd.psoDesc.DepthEnable = FALSE;
	cmd.psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

	// メッシュ（三角形）情報の設定
	cmd.vbView = modelData_->vbView;
	cmd.ibv = modelData_->ibView;
	cmd.indexCount = modelData_->indexCount;

	// バインドレスマテリアルのインデックスとテクスチャインデックスを設定
	cmd.materialIndex = GetMaterialDescriptorIndex();
	cmd.textureIndex = textureIndex_;
	cmd.layer = layer_;

	// シングルトンではない RenderSystem インスタンスに直接コマンドを積む
	renderSystem->PushCommand(cmd);
}