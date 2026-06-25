#include "PCH.h"
#include "GameObject.h"
#include "Component.h"
#include "imgui.h"
#include "BaseScene.h"
#include "ComponentType.h"

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
		std::string newName = nameBuf;
		// 空文字やスペース/タブだけの名前は反映させない（元の名前を維持する）
		if (!newName.empty() && newName.find_first_not_of(" \t\r\n") != std::string::npos) {
			name_ = newName;
		}
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

		// Light の追加メニュー
		if(GetComponent<LightComponent>() == nullptr) {
			if(ImGui::MenuItem("Light")) {
				auto* newComp = AddComponent<LightComponent>();
				newComp->Initialize();
			}
		}
		else {
			ImGui::TextDisabled("Light (Already Added)");
		}

		// コンポーネントが増えたらここに
		ImGui::EndPopup();
	}
}

json GameObject::Serialize() const {
	json j;
	j["name"] = name_;

	// Transform情報の書き出し（ラジアンのまま直で保存！）
	j["transform"]["position"] = { transform_.translate.x, transform_.translate.y, transform_.translate.z };
	j["transform"]["rotation"] = { transform_.rotate.x, transform_.rotate.y, transform_.rotate.z };
	j["transform"]["scale"]    = { transform_.scale.x, transform_.scale.y, transform_.scale.z };

	// 各コンポーネントのSerializeを呼ぶ
	j["components"] = json::array();
	for (const auto& comp : components_) {
		json compJ;
		comp->Serialize(compJ);
		j["components"].push_back(compJ);
	}

	return j;
}

void GameObject::Deserialize(const json& j) {
	name_ = j["name"];

	// Transformの復元（ラジアンのまま直で復元！）
	if (j.contains("transform")) {
		auto pos = j["transform"]["position"];
		transform_.translate = { pos[0], pos[1], pos[2] };

		auto rot = j["transform"]["rotation"];
		transform_.rotate = { rot[0], rot[1], rot[2] };

		auto scale = j["transform"]["scale"];
		transform_.scale = { scale[0], scale[1], scale[2] };
	}

	// コンポーネントリストの復元
	if (j.contains("components")) {
		for (const auto& compJ : j["components"]) {
			std::string type = compJ["type"];

			if (type == "MeshRendererComponent") {
				// 二重追加を防ぐチェックをしてから追加
				auto* comp = GetComponent<MeshRendererComponent>();
				if (!comp) {
					comp = AddComponent<MeshRendererComponent>();
				}
				comp->Deserialize(compJ);
			}
			else if (type == "LightComponent") {
				auto* comp = GetComponent<LightComponent>();
				if (!comp) {
					comp = AddComponent<LightComponent>();
				}
				comp->Deserialize(compJ);
			}
		}
	}
}
