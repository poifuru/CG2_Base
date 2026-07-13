#pragma once
#include <Windows.h>
#include <Wrl.h>
#include <d3d12.h>
#include <string>
#include <memory>
#include "struct.h"
#include "PSOManager.h"
#include "Component.h"
#include "ConstantBuffer.h"
#include "TransformMatrixData.h"

// 前方宣言
namespace MyEngine::Rendering {
	struct PSODescriptor;
}

struct SkyboxVertex {
	Vector4 position;
};

class SkyboxComponent : public Component {
public:
	SkyboxComponent() = default;
	~SkyboxComponent() override;

	void Initialize() override;
	void Update() override;
	void Draw(MyEngine::Rendering::Renderer* renderer) override;

	void SetTexture(const std::string& filePath);
	const std::string& GetTexturePath() const { return texturePath_; }

	void ImGui() override;

	void Serialize(json& j) const override;
	void Deserialize(const json& j) override;

	const char* GetName() const override { return "SkyboxComponent"; }

private:
	// PSO
	MyEngine::Rendering::PSODescriptor desc_{};

	// バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbv_{};
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer_ = nullptr;
	D3D12_INDEX_BUFFER_VIEW ibv_{};
	ConstantBuffer<TransformMatrixData> matrixBuffer_;

	// マッピング用のポインタ
	SkyboxVertex* vertexData_ = nullptr;
	uint32_t* indexData_ = nullptr;

	// シェーダーとテクスチャインデックス
	uint32_t vsID_ = 0;
	uint32_t psID_ = 0;
	uint32_t textureIndex_ = 0;
	std::string texturePath_;

	std::shared_ptr<class Material> material_ = nullptr;

	// リセット防止
	bool isInitialized_ = false;
};