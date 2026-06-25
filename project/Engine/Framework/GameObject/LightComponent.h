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
	Vector3 position;	// 位置
	float	radius;	//ライトが届く最大距離
	float	decay;		//減衰率
};

// SpotLight
struct Spot {
	Vector4 color;		// 色
	float	intensity;	// 強度
	Vector3 position;	// 位置
	Vector3 direction;	// 向き
	float	distance;	// ライトが届く最大距離
	float	decay;		// 減衰率
	float	cosAngle;	// スポットライトの余弦
};

// RectLight
struct Rect {
	Vector4 color;		// 色
	float	intensity;	// 強度
	Vector3 position;	// 位置
	Vector3 direction;	// 向き
	Vector2 size;		// 大きさ
	Vector3 right;		// 右ベクトル
	Vector3 up;		// 上ベクトル
	float	decay;		// 減衰率
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

#pragma region アクセッサ (LightManagerがデータを集約するときに使う)
	// LightTypeを返す
	LightType GetLightType() const { return type_; }

	// Colorを返す
	Vector4 GetColor() const;
	// Colorをセット
	void SetColor(const Vector4& color);

	// Intensityを返す
	float GetIntensity() const;
	// Intensityをセット
	void SetIntensity(const float& intensity);

	// Positionを返す
	Vector3 GetPosition() const;
	// Positionをセット
	void SetPosition(const Vector3& position);

	// Directionを返す
	Vector3 GetDirection() const;
	// Directionをセット
	void SetDirection(const Vector3& direction);

	// Radiusを返す
	float GetRadius() const;
	// Radiusをセット
	void SetRadius(const float& radius);

	// Decayを返す
	float GetDecay() const;
	// Decayをセット
	void SetDecay(const float& decay);

	// Distanceを返す
	float GetDistance() const;
	// Distanceをセット
	void SetDistance(const float& distance);

	// CosAngleを返す
	float GetCosAngle() const;
	// CosAngleをセット
	void SetCosAngle(const float& cosAngle);

	// Sizeを返す
	Vector2 GetSize() const;
	// Sizeをセット
	void SetSize(const Vector2& size);

	// --- 構造体を丸ごと返す --- //
	// Directional構造体を返す
	const Directional* GetDirectionalParam() const;

	// Point構造体を返す
	const Point* GetPointParam() const;

	// Spot構造体を返す
	const Spot* GetSpotParam() const;

	// Rect構造体を返す
	const Rect* GetRectParam() const;
#pragma endregion

private:
	LightType type_ = DIRECTIONALLIGHT;
	std::variant<Directional, Point, Spot, Rect> param_;
	bool isInitialized_ = false;
};