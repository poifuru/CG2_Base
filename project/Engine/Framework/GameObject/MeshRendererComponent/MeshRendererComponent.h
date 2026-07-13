#pragma once
#include "Component.h"
#include <memory>
#include <string>
#include "Model.h"

namespace MyEngine::Rendering {
	class Renderer;
}

class MeshRendererComponent : public Component {
public:
	MeshRendererComponent() = default;
	~MeshRendererComponent() override = default;

	void Initialize() override;
	void Update() override;
	void Draw(MyEngine::Rendering::Renderer* renderer) override;
	void ImGui() override;

	void Serialize(json& j) const override;
	void Deserialize(const json& j) override;

	const char* GetName() const override { return "Mesh Renderer"; }

	// モデル・テクスチャを変更する関数
	void SetModel(const std::string& modelPath);
	void SetTexture(const std::string& textureName);

	// マテリアルカラーを直接変更する関数
	void SetColor(const Vector4& color);

	// ライティングの有効・無効を切り替える関数
	void SetEnableLighting(bool flag);

	// ブレンドモードを設定する関数
	void SetBlendMode(MyEngine::Rendering::BlendModeType mode);

	// デプスの有効・無効を設定する関数
	void SetDepthEnable(bool flag);

	// レンダリングレイヤーを設定する関数
	void SetLayer(uint8_t layer);

	// 両面表示を設定する関数
	void SetDoubleSided(bool flag);

private:
	std::unique_ptr<Model> model_ = nullptr;
	std::string modelPath_ = "Resources/plane/plane.obj";	// デフォルトモデル
	std::string texPath_ = "Resources/uvChecker.png";		// デフォルトテクスチャ
	uint32_t vsID_ = 0;
	uint32_t psID_ = 0;
	uint32_t modelIndex_ = 0;
	uint32_t texIndex_ = 0;
	bool isDepthEnable_ = true;
};