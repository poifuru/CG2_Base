#include "GameObject.h"
#include "Component.h"
#include "imgui.h"
#include "BaseScene.h"
#include "MeshRendererComponent.h"

GameObject::GameObject(SceneContext* context, const std::string& name)
	: context_(context), name_(name) {
	// トランスフォームの初期化
	transform_.scale = { 1.0f, 1.0f, 1.0f };
	transform_.rotate = { 0.0f, 0.0f, 0.0f };
	transform_.translate = { 0.0f, 0.0f, 0.0f };
}

GameObject::~GameObject() = default;

void GameObject::Initialize() {
	for (auto& component : components_) {
		component->Initialize();
	}
}

void GameObject::Update() {
	for (auto& component : components_) {
		component->Update();
	}
}

void GameObject::Draw(class RenderSystem* renderSystem) {
	for (auto& component : components_) {
		component->Draw(renderSystem);
	}
}

void GameObject::ImGui() {
	// オブジェクト名の編集
	char nameBuf[128];
	strcpy_s(nameBuf, name_.c_str());
	if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
		name_ = nameBuf;
	}

	ImGui::Separator();

	// Transformの編集（デフォルトで常に表示）
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::DragFloat3("Position", &transform_.translate.x, 0.1f);
		ImGui::DragFloat3("Rotation", &transform_.rotate.x, 0.1f);
		ImGui::DragFloat3("Scale", &transform_.scale.x, 0.1f);
	}

	// 各コンポーネントのImGui描画（折りたたみヘッダーで表示）
	for (auto& component : components_) {
		if (ImGui::CollapsingHeader(component->GetName(), ImGuiTreeNodeFlags_DefaultOpen)) {
			component->ImGui();
		}
	}

	ImGui::Separator();
	ImGui::Spacing();

	// 「Add Component」ボタンを表示
	if (ImGui::Button("Add Component", ImVec2(-1, 30))) {
		ImGui::OpenPopup("AddComponentPopup");
	}

	// ボタンが押されたらポップアップメニューを開く
	if(ImGui::BeginPopup("AddComponentPopup")) {

		// Mesh Renderer の追加メニュー
		// MeshRendererComponent を持っていない場合だけ選択できるようにする
		if(GetComponent<MeshRendererComponent>() == nullptr) {
			if(ImGui::MenuItem("Mesh Renderer")) {
				// コンポーネントを追加して、すぐに初期化を呼ぶ
				auto* newComp = AddComponent<MeshRendererComponent>();
				newComp->Initialize();
			}
		}
		else {
			// すでに追加されている場合はグレーアウト表示
			ImGui::TextDisabled("Mesh Renderer (Already Added)");
		}
		// コンポーネントが増えたらここに
		ImGui::EndPopup();
	}
}