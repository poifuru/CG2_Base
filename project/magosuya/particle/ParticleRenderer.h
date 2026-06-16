#pragma once
#include <Windows.h>
#include <wrl.h>
using namespace Microsoft::WRL;
#include <d3d12.h>
#include <map>
#include <vector>
#include "struct.h"
#include "PSOManager.h"

//パーティクルの種類
enum class ParticleType {
	Standard,
	Mesh
};

//GPUに送るParticleのデータ
struct ParticleForGPU {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};

struct ParticleRenderData {
	//頂点、インデックスバッファー
	ComPtr<ID3D12Resource> vertexBuffer = nullptr;
	VertexData* vertexData_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vbView;

	ComPtr<ID3D12Resource> indexBuffer = nullptr;
	uint32_t* indexData_ = nullptr;
	D3D12_INDEX_BUFFER_VIEW ibView;

	//描画用バッファー(instancing)
	ComPtr<ID3D12Resource> instancingBuffer_ = nullptr;
	ParticleForGPU* instancingData_ = nullptr;

	//マテリアルバッファー
	ComPtr<ID3D12Resource> materialBuffer_ = nullptr;
	Material* materialData_ = nullptr;

	//描画できる最大数
	uint32_t kMaxParticleNum = 1000;
	uint32_t currentCount = 0;

	// このグループで使うPSO
	PSODescriptor psoDesc;
};

class ParticleRenderer {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="type">使いたいパーティクルの種類</param>
	void Initialize(ParticleType type);

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="vpMatrix">カメラ行列</param>
	void Update(const Matrix4x4& vpMatrix);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

private:
	//描画設定
	ParticleRenderData* data_ = nullptr;
};