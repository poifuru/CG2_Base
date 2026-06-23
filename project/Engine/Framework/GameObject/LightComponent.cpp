#include "LightComponent.h"
#include "GameObject.h"
#include "MathFunction.h"

void LightComponent::Initialize() {

}

void LightComponent::Update() {
	GameObject* owner = GetGameObject();
	if(!owner) return;

	// Transformの取得
	auto& transform = owner->GetTransform();

	// 位置の同期
	Vector3 lightPosition = transform.translate;
}

void LightComponent::ImGui() {

}

void LightComponent::Serialize(json& j) const {

}

void LightComponent::Deserialize(const json& j) {

}
