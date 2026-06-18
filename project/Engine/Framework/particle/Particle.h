#pragma once
#include <Windows.h>
#include <Wrl.h>
using namespace Microsoft::WRL;
#include <d3d12.h>
#include <vector>
#include <list>
#include <memory>
#include <random>
#include "struct.h"
#include "PSOManager.h"
#include "SRVManager.h"
#include "ParticleRenderer.h"

struct ParticleData {
	EulerTransform transform;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;
};

class DxCommon;

class Particle {
public:
	Particle (DxCommon* dxCommon);
	~Particle ();

	void Initialize ();
	void Update (Matrix4x4* cameraMatrix, Matrix4x4* vp);
	void Draw ();
	void ImGui ();

	uint32_t GetParticleNum () { return kMaxParticleNum_; };
	void SetTexHandle (D3D12_GPU_DESCRIPTOR_HANDLE handle) { handle_ = handle; }

	// マリンスノー機能を有効にするか
	void SetMarineSnow(bool active) { isMarineSnow_ = active; }
	bool IsMarineSnow() const { return isMarineSnow_; }

private:	//内部関数
	ParticleData MakeNewParticle (std::mt19937 randomEngine, const Emitter& emitter_);
	ParticleData MakeNewMarineSnow (std::mt19937 randomEngine, const Vector3& cameraPos);
	std::list<ParticleData> Emit (const Emitter& emitter, std::mt19937& randomEngine);
	void EmitterUpdate ();
	void EmitterUpdateMarineSnow (const Vector3& cameraPos);

private:
	//PSOの設定
	PSODescriptor desc_ = {};

	//モデルデータ
	std::unique_ptr<ModelData> data_ = nullptr;
	//何個のパーティクルを出すのか
	const uint32_t kMaxParticleNum_ = 500;
	uint32_t numInstance_ = 0; //描画すべきインスタンス数

	//GPUリソース
	ComPtr<ID3D12Resource> instancingBuffer_;
	ComPtr<ID3D12Resource> materialBuffer_;

	//マッピング用のCPUデータ
	VertexData* vertexData_ = nullptr;
	uint32_t* indexData_ = nullptr;
	ParticleForGPU* instancingData_ = nullptr;
	Material* materialData_ = nullptr;

	//ディスクリプタハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE particleSrvHandleCPU = {};
	D3D12_GPU_DESCRIPTOR_HANDLE particleSrvHandleGPU = {};
	D3D12_GPU_DESCRIPTOR_HANDLE handle_ = {};

	//位置データ
	std::list<ParticleData> particles_;
	std::list<ParticleData>::iterator particleIterator_;
	EulerTransform uvTransform_;
	Emitter emitter_ = {};

	//速度をランダムに割り当てるための乱数生成器
	//生成エンジンの型を作る
	using RNG_Engine = std::mt19937;
	//作った型でエンジンの宣言
	RNG_Engine randomEngine_;
	//実行ごとに異なる値を取得する
	std::random_device rd;
	//分布
	std::uniform_real_distribution<float> pos_x;			//発生位置_x
	std::uniform_real_distribution<float> pos_y;			//発生位置_y
	std::uniform_real_distribution<float> pos_z;			//発生位置_z
	std::uniform_real_distribution<float> rand_;		//速度
	std::uniform_real_distribution<float> randColor_;	//色
	std::uniform_real_distribution<float> randTime_;	//パーティクルの生存可能時間

	//ビルボードの変数
	bool useBillBoard = false;
	Matrix4x4 billBoardMatrix_ = {};

	//ImGui用の変数
	int currentBlendMode_ = static_cast<int>(BlendModeType::Additive); // 現在のブレンドモードのインデックス
	const char* blendModeNames_[6] = { "Opaque", "Alpha", "Additive", "Subtract", "Multiply", "Screen" };
	const int kBlendModeCount_ = 6;

	// マリンスノー用パラメータ
	bool isMarineSnow_ = false;               // マリンスノーモードのフラグ
	Vector3 marineSnowRange_ = { 30.0f, 20.0f, 30.0f }; // カメラ周囲の発生/存在範囲 (幅, 高さ, 奥行き)
	float marineSnowFallSpeed_ = 0.5f;        // 落下速度
	float marineSnowDriftSpeed_ = 5.0f;       // ゆらゆら揺れる速度（周波数）
	float marineSnowDriftScale_ = 1.0f;       // ゆらゆら揺れる幅（振幅）
	float marineSnowNearFadeLimit_ = 2.0f;    // カメラにこれ以上近づいたらフェードアウトする距離
	float marineSnowMinSize_ = 0.001f;         // 粒の最小サイズ
	float marineSnowMaxSize_ = 0.003f;          // 粒の最大サイズ

	//ポインタを借りる
	DxCommon* dxCommon_ = nullptr;
	ID3D12Device* device_ = nullptr;
	ID3D12GraphicsCommandList* commandList_ = nullptr;
	SRVManager* srvManager_ = nullptr;
};

