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

//const Vector4& LightComponent::GetColor() const {
//	if(auto* p = std::get_if<Directional>(&param_)) return p->color;
//	if(auto* p = std::get_if<Point>(&param_)) return p->color;
//	if(auto* p = std::get_if<Spot>(&param_)) return p->color;
//	if(auto* p = std::get_if<Rect>(&param_)) return p->color;
//
//	// どれでもない場合のデフォルト値
//	return { 1.0f, 1.0f, 1.0f, 1.0f };
//}
//
//const float& LightComponent::GetIntensity() const {
//	if(auto* p = std::get_if<Directional>(&param_)) return p->intensity;
//	if(auto* p = std::get_if<Point>(&param_)) return p->intensity;
//	if(auto* p = std::get_if<Spot>(&param_)) return p->intensity;
//	if(auto* p = std::get_if<Rect>(&param_)) return p->intensity;
//
//	// どれでもない場合のデフォルト値
//	return 0.0f;
//}

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
