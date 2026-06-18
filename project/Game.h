#pragma once
#include <Windows.h>
#include <Windows.h>
#include <memory>
#include <vector>
#include <DirectXMath.h>
#include "Engine.h"

// 必要なバッファクラスのヘッダーをインクルード
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "StructuredBuffer.h"
#include "SceneManager.h"

class Game {
public:
	Game();
	~Game();

	void Run();

private:
	// 頂点構造体の定義（POSITION, TEXCOORD, NORMAL）
	struct Vertex {
		DirectX::XMFLOAT4 pos;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT3 normal;
	};

	// マテリアル構造体の定義（シェーダー側の本棚の中身と一致させる）
	struct MaterialData {
		DirectX::XMFLOAT4 color;
		uint32_t textureIndex;
		uint32_t padding[3]; // 16バイトアライメント用のパディング
	};

private:
	std::unique_ptr<Engine> engine_ = nullptr;
	//std::unique_ptr<SceneManager> sceneManager_ = nullptr;

	// 三角形描画用のデータ構造バッファ
	VertexBuffer<Vertex> vertexBuffer_;
	IndexBuffer<uint32_t> indexBuffer_;
	StructuredBuffer<MaterialData> materialBuffer_;

	// シェーダーID
	uint32_t vsID_ = 0;
	uint32_t psID_ = 0;
};