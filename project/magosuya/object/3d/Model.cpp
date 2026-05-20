#include "Model.h"
#include "DeltaTime.h"

Model::Model(DxCommon* dxCommon, LightManager* lightManager)
	: BaseObject3d(dxCommon, lightManager){

}

Model::~Model() {

}

void Model::Initialize(ModelData* modelData, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) {
	// 親クラスの初期化
	BaseObject3d::Initialize();

	// === Modelクラス自身の初期化 === //
	// 工場から貰ったアセットをそのままセット！
	modelData_ = modelData;
	textureHandle_ = textureHandle; // 親クラス（BaseObject3d）のメンバに格納

	// このモデルが使うPSOやテクスチャのデフォルトをセット
	rootSignatureID_ = RootSignatureManager::GetInstance()->GetOrCreateRootSignature(RootSigType::Standard3D); // スキニングなし用の標準ルートシグネチャIDなど
	psoDesc_.VS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/SkinningObject3d.VS.hlsl", L"vs_6_0");
	psoDesc_.PS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/SkinningObject3d.PS.hlsl", L"ps_6_0");
	psoDesc_.InputLayoutID = InputLayoutType::Standard3D;
	psoDesc_.BlendMode = BlendModeType::Alpha;
}

void Model::Update(CameraData* cameraData) {
	// Animationがセットされていればタイマーカウント
	if(animation_) {
		animationTime_ += kDeltaTime;
	}

	BaseObject3d::Update(cameraData);
}

void Model::Draw() {
	if(!modelData_) return;

	// RenderCommandの組み立て
	RenderCommand cmd{};

	// PSOの設定
	cmd.rootSignatureID = rootSignatureID_;
	cmd.psoDesc = psoDesc_;

	// メッシュ情報
	cmd.vbViews[0] = modelData_->vbView;
	cmd.ibv = modelData_->ibView;
	cmd.indexCount = modelData_->indexCount;

	// 定数バッファのアドレス
	cmd.transformCBV = transformResource_.get()->GetGPUVirtualAddress();
	cmd.materialCBV = materialResource_.get()->GetGPUVirtualAddress();
	cmd.textureSRV = textureHandle_;

	// 不透明
	cmd.layer = 0; 

	// コマンドを投げる
	RenderSystem::GetInstance()->PushCommand(cmd);
}
