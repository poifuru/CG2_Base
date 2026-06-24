#pragma once
#include "Component.h"
#include "LightManager.h"
#include "struct.h"

//　--- 構造体の定義 --- //
// DirectionalLight
struct Directional {
	Vector4	color;		// 色
	float	intensity;	// 強度
	Vector3 direction;	// 向き
};

// PointLight
struct Point {
	Vector4 color;		// 色
	float	intensity;	// 強度
	Vector3 position_;	// 位置
	float	radius_;	//ライトが届く最大距離
	float	decay_;		//減衰率
};

// SpotLight
struct Spot {
	Vector4 color;		// 色
	float	intensity;	// 強度
	Vector3 position_;	// 位置
	Vector3 direction_;	// 向き
	float	distance_;	// ライトが届く最大距離
	float	decay_;		// 減衰率
	float	cosAngle_;	// スポットライトの余弦
};

// RectLight
struct Rect {
	Vector4 color;		// 色
	float	intensity;	// 強度
	Vector3 position_;	// 位置
	Vector3 direction_;	// 向き
	Vector2 size_;		// 大きさ
	Vector3 right_;		// 右ベクトル
	Vector3 up_;		// 上ベクトル
	float	decay_;		// 減衰率
};

class LightComponent : public Component {
public:
	LightComponent() = default;
	~LightComponent() override = default;

	void Initialize() override;
	void Update() override;
	void ImGui() override;

	void Serialize(json& j) const override;
	void Deserialize(const json& j) override;

	const char* GetName() const override { return "Light"; }

	// ゲッター・セッター（LightManagerがデータを集約するときに使う）
	LightType GetLightType() const { return type_; }

	// Colorを返す
	const Vector4* GetColor() const;

	// Directional構造体を返す
	const Directional* GetDirectionalParam() const;

	// Point構造体を返す
	const Point* GetPointParam() const;

	// Spot構造体を返す
	const Spot* GetSpotParam() const;

	// Rect構造体を返す
	const Rect* GetRectParam() const;

private:
	LightType type_ = DIRECTIONALLIGHT;
	std::variant<Directional, Point, Spot, Rect> param_;
};