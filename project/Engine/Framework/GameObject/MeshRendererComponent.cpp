#include "MeshRendererComponent.h"
#include "GameObject.h"
#include "BaseScene.h"       // SceneContextを使うため
#include "ModelFactory.h"
#include "ModelManager.h"
#include "ShaderManager.h"
#include "CameraOrganizer.h" // カメラ情報を取得するため
#include "imgui.h"

void MeshRendererComponent::Initialize() {
	// 初期モデルのロード
	SetModel(modelPath_);
}
void MeshRendererComponent::Update() {
	if(!model_ || !gameObject_) return;

	// カメラのデータを取得してモデルをアップデート
	CameraData& cameraData = CameraOrganizer::GetInstance()->GetCameraData();
	model_->Update(&cameraData);
}

void MeshRendererComponent::Draw(RenderSystem* renderSystem) {
	if (!model_) return;

	// モデルの描画
	model_->Draw(renderSystem);
}

void MeshRendererComponent::ImGui() {
	// モデルパスの表示と変更（簡易的なInputText）
	char pathBuf[256];
	strcpy_s(pathBuf, modelPath_.c_str());
	if (ImGui::InputText("Model Path", pathBuf, sizeof(pathBuf), ImGuiInputTextFlags_EnterReturnsTrue)) {
		SetModel(pathBuf);
	}
	ImGui::Spacing();

	// 既存のModelクラスが持っているImGuiの調整機能もそのまま呼べる
	if(model_) {
		model_->ImGui("Model Material");
	}
}

void MeshRendererComponent::Serialize(json& j) const {
	j["type"] = "MeshRendererComponent";
	j["modelPath"] = modelPath_;
}
void MeshRendererComponent::Deserialize(const json& j) {
	if (j.contains("modelPath")) {
		SetModel(j["modelPath"]);
	}
}

void MeshRendererComponent::SetModel(const std::string& modelPath) {
	GameObject* owner = GetGameObject();
	if (!owner) return;
	SceneContext* context = owner->GetContext();
	if (!context) return;
	modelPath_ = modelPath;

	// シェーダーのコンパイル＆キャッシュ（既存のシェーダーを使用）
	vsID_ = context->shaderManager->CompileAndCacheShader(L"Resources/shader/Object3d.VS.hlsl", L"vs_6_0");
	psID_ = context->shaderManager->CompileAndCacheShader(L"Resources/shader/Object3d.PS.hlsl", L"ps_6_0");
	
	// モデルデータのロード
	modelIndex_ = context->modelManager->LoadModelData(modelPath_);
	
	// モデルの生成（一旦リセットしてから再生成）
	model_.reset();
	model_ = context->modelFactory->CreateModel(vsID_, psID_, modelIndex_, 0);

	if (model_ && gameObject_) {
		model_->SetParentTransform(&gameObject_->GetTransform());
	}
}