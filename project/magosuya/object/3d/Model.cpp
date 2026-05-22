#include "Model.h"
#include "DeltaTime.h"
#include "MathFunction.h"

Model::Model(DxCommon* dxCommon)
	: BaseObject3d(dxCommon){

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
	psoDesc_.RootSignatureID = RootSignatureManager::GetInstance()->GetOrCreateRootSignature(RootSigType::Standard3D); // スキニングなし用の標準ルートシグネチャIDなど
	psoDesc_.VS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/Object3d.VS.hlsl", L"vs_6_0");
	psoDesc_.PS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/Object3d.PS.hlsl", L"ps_6_0");
	psoDesc_.InputLayoutID = InputLayoutType::Standard3D;
	psoDesc_.BlendMode = BlendModeType::Alpha;

	layer_ = 1;
}

void Model::Update(CameraData* cameraData) {
	// Animationがセットされていればタイマーカウント
	if(animation_) {
		animationTime_ += kDeltaTime;
	}

	//// Animationの再生
	//animationTime_ += 1.0f / 60.0f;
	//animationTime_ = std::fmod(animationTime_, data->duration);	// 最後まで行ったらリピート

	//Vector3 translate = modelData_->rootNode.transform.translate;
	//Quaternion rotate = modelData_->rootNode.transform.rotate;
	//Vector3 scale = modelData_->rootNode.transform.scale;

	//if(auto it = data->nodeAnimations.find(modelData_->rootNode.name); it != data->nodeAnimations.end()) {
	//	const NodeAnimation& rootNodeAnimation = (*it).second;
	//	if(!rootNodeAnimation.translate.keyframes.empty()) {
	//		translate = CalculateValue(rootNodeAnimation.translate.keyframes, animationTime_);
	//	}
	//	if(!rootNodeAnimation.rotate.keyframes.empty()) {
	//		rotate = CalculateValue(rootNodeAnimation.rotate.keyframes, animationTime_);
	//	}
	//	if(!rootNodeAnimation.scale.keyframes.empty()) {
	//		scale = CalculateValue(rootNodeAnimation.scale.keyframes, animationTime_);
	//	}
	//}

	//return Math::MakeAffineMatrix(scale, rotate, translate);

	BaseObject3d::Update(cameraData);
}

void Model::Draw() {
	if(!modelData_) return;

	// RenderCommandの組み立て
	RenderCommand cmd{};

	// PSOの設定
	cmd.rootSignatureID = psoDesc_.RootSignatureID;
	cmd.psoDesc = psoDesc_;

	// メッシュ情報
	cmd.vbViews[0] = modelData_->vbView;
	cmd.ibv = modelData_->ibView;
	cmd.indexCount = modelData_->indexCount;

	// 定数バッファのアドレス
	cmd.binds[3].type = BindingType::CBV;
	cmd.binds[3].gpuAddress = transformBuffer_.get()->GetGPUVirtualAddress();

	cmd.binds[4].type = BindingType::CBV;
	cmd.binds[4].gpuAddress = materialBuffer_.get()->GetGPUVirtualAddress();

	cmd.binds[5].type = BindingType::SRV_Table;
	cmd.binds[5].descriptorHandle = textureHandle_;

	// 透明
	cmd.layer = layer_;

	// 描画タイプ
	cmd.renderType = renderType_;

	// コマンドを投げる
	RenderSystem::GetInstance()->PushCommand(cmd);
}