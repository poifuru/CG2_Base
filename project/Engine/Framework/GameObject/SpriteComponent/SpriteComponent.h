#pragma once
#include "Component.h"

namespace MyEngine::Rendering {
	class Renderer;
	class Model;
	class Material;
}

class SpriteComponent : public Component {
public:
	SpriteComponent();
	~SpriteComponent() override;

	void Initialize() override;
	void Update() override;
	void ImGui() override;

	void Serialize(json& j) const override;
	void Deserialize(const json& j) override;

	const char* GetName() const override { return "Sprite Component"; }

	MyEngine::Rendering::Model* GetModel() const;

	MyEngine::Rendering::Material* GetMaterial() const;

	// テクスチャ設定
	void SetTexture(const std::string& textureName);

	// 各種パラメータへのアクセッサ
	void SetPosition(const Vector2& pos) { position_ = pos; }
	const Vector2& GetPosition() const { return position_; }

	void SetSize(const Vector2& size) { size_ = size; }
	const Vector2& GetSize() const { return size_; }

	void SetRotation(float rot) { rotation_ = rot; }
	float GetRotation() const { return rotation_; }

	void SetScale(const Vector2& scale) { scale_ = scale; }
	const Vector2& GetScale() const { return scale_; }

	void SetColor(const Vector4& color);
	const Vector4& GetColor() const { return color_; }

	void SetAnchorPoint(const Vector2& anchor) { anchorPoint_ = anchor; }
	const Vector2& GetAnchorPoint() const { return anchorPoint_; }

	void SetLayer(uint8_t layer) { layer_ = layer; }
	uint8_t GetLayer() const { return layer_; }

private:
	std::unique_ptr<MyEngine::Rendering::Model> model_ = nullptr;
	std::string texPath_ = "Resources/uvChecker.png"; // デフォルトテクスチャ
	std::string modelPath_ = "Resources/plane/plane.obj"; // デフォルトモデル

	Vector2 position_ = { 100.0f, 100.0f }; // デフォルト位置
	Vector2 size_ = { 100.0f, 100.0f };     // デフォルトサイズ
	float rotation_ = 0.0f;                  // デフォルト角度（ラジアン）
	Vector2 scale_ = { 1.0f, 1.0f };         // デフォルトスケール
	Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f }; // デフォルト色
	Vector2 anchorPoint_ = { 0.5f, 0.5f };   // デフォルトアンカーポイント(中央)
	uint8_t layer_ = 4;                      // UI用のレイヤー

	uint32_t modelIndex_ = 0;
	uint32_t texIndex_ = 0;
};
