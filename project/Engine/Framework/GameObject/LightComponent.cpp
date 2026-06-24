#include "PCH.h"
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

const Vector4 LightComponent::GetColor() const {
	if(auto* p = std::get_if<Directional>(&param_)) return p->color;
}

const Directional* LightComponent::GetDirectionalParam() const {
	return std::get_if<Directional>(&param_);
}

const Point* LightComponent::GetPointParam() const {
	return std::get_if<Point>(&param_);
}

const Spot* LightComponent::GetSpotParam() const {
	return std::get_if<Spot>(&param_);
}

const Rect* LightComponent::GetRectParam() const {
	return std::get_if<Rect>(&param_);
}
