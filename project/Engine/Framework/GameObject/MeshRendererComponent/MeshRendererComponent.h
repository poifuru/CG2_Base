#pragma once
#include "Component.h"
#include <memory>
#include <string>
#include "Model.h"

class RenderSystem;

class MeshRendererComponent : public Component {
public:
	MeshRendererComponent() = default;
	~MeshRendererComponent() override = default;

	void Initialize() override;
	void Update() override;
	void Draw(RenderSystem* renderSystem) override;
	void ImGui() override;

	void Serialize(json& j) const override;
	void Deserialize(const json& j) override;

	const char* GetName() const override { return "Mesh Renderer"; }

	// モデル・テクスチャを変更する関数
	void SetModel(const std::string& modelPath);
	void SetTexture(const std::string& textureName);

private:
	std::unique_ptr<Model> model_ = nullptr;
	std::string modelPath_ = "Resources/plane/plane.obj";	// デフォルトモデル
	std::string texPath_ = "Resources/uvChecker.png";		// デフォルトテクスチャ
	uint32_t vsID_ = 0;
	uint32_t psID_ = 0;
	uint32_t modelIndex_ = 0;
	uint32_t texIndex_ = 0;
	bool isInitialized_ = false;
	bool isDepthEnable_ = true;
};