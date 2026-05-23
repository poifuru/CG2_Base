#include "Model.h"
#include "DeltaTime.h"
#include "MathFunction.h"
#include "imgui.h"
#include "Animator.h"

Model::Model(DxCommon* dxCommon)
	: BaseObject3d(dxCommon) {

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
	animator_->Update();

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

	if(animator_->IsSkinning() && renderType_ == RenderType::Skining) {
		cmd.vbViews[1] = animator_->GetInfluenceVBV();

		cmd.binds[6].type = BindingType::SRV_Table;
		cmd.binds[6].descriptorHandle = animator_->GetSkinCluster().GetPaletteSRVHandle();
	}

	// 透明
	cmd.layer = layer_;

	// 描画タイプ
	cmd.renderType = renderType_;

	// コマンドを投げる
	RenderSystem::GetInstance()->PushCommand(cmd);
}

void Model::ImGui(const std::string& label) {
	// ラベル作成
	std::string objLabel = "##" + label + std::to_string(instanceID_);

	if(ImGui::TreeNode((label + objLabel).c_str())) {
		BaseObject3d::ImGui(objLabel);

		if(animator_ != nullptr) {
			bool animationFlag = animator_->IsAnimation();
			if(ImGui::TreeNode(("Animation" + objLabel).c_str())) {
				ImGui::Checkbox(("IsActive" + objLabel).c_str(), &animationFlag);
				// タイムライン用のスライダー
					// 時間をフレーム数に変換（1秒=60フレームとした場合）
				int currentFrame = static_cast<int>(animator_->GetAnimation()->animationTime * 60.0f);
				int maxFrame = static_cast<int>(animator_->GetAnimation()->duration * 60.0f);

				if(ImGui::SliderInt(("Frame" + objLabel).c_str(), &currentFrame, 0, maxFrame, "%d")) {
					// 操作されたら、フレーム数を秒数に戻して代入
					animator_->GetAnimation()->animationTime = static_cast<float>(currentFrame) / 60.0f;
					animationFlag = false;
				}
				animator_->SetIsAnimation(animationFlag);
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
}

void Model::SetAnimator(Animator* animator) {
	animator_ = animator;
}

Matrix4x4 Model::CalculateWorldMatrix() {
	// まず通常のトランスフォーム（ImGuiとかで動かす用）の行列を作る
	Matrix4x4 affine = Math::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	// アニメーションがあるなら、そのローカル行列を計算して掛け合わせる
	if(animator_) {
		Matrix4x4 animationMatrix = animator_->GetRootAnimationMatrix(); // 時間更新を除いた純粋な行列計算に変更
		return animationMatrix * affine;
	}

	// アニメーションがないなら通常のアフィン変換だけ
	return affine;
}