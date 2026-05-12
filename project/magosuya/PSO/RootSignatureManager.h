#pragma once
#include <Windows.h>
#include <Wrl.h>
using namespace Microsoft::WRL;
#include <d3d12.h>
#include <string>
#include <unordered_map>

class DxCommon;

//使いたい用途によって設定を変えるため
enum class RootSigType : uint32_t {
	Standard3D,			//基本的な3D描画物用
	Standard3DInstance,	//インスタンス描画用
	SkinningStandard3D, //スキニング対応の3D描画用
	Sprite,				//スプライト用
	Particle,			//パーティクル用
	LineMesh,
	CubeMesh,			//メッシュ描画用
	PostProcess,		// ポストエフェクト用（SRV中心）(未実装)
	UI,					// UI描画用（2D行列とテクスチャ）(未実装)
	Skybox,				// Skybox用
	Count,
};

class RootSignatureManager {
public:		// メンバ関数
	static RootSignatureManager* GetInstance () {
		//初めて呼び出されたときに一回だけ初期化
		static RootSignatureManager instance;
		return &instance;
	}

	void Initialize (DxCommon* dxCommon);

	//ルートシグネチャの定義を受け取って、生成&キャッシュしてIDを返す
	//(後で定義情報を含む独自のDescriptor構造体を作って差し替え)
	uint32_t GetOrCreateRootSignature (RootSigType type);

	//IDをもとにID3D12RootSignature*を返す
	ID3D12RootSignature* GetRootSignature (uint32_t rootSigID) const;

	void SetRootSignature (uint32_t rootSigID);

private:
	//コンストラクタを禁止
	RootSignatureManager () = default;
	// コピーコンストラクタと代入演算子を禁止
	RootSignatureManager (const RootSignatureManager&) = delete;
	RootSignatureManager& operator=(const RootSignatureManager&) = delete;
	RootSignatureManager (RootSignatureManager&&) = delete;
	RootSignatureManager& operator=(RootSignatureManager&&) = delete;

private:	// ヘルパー関数
	D3D12_ROOT_SIGNATURE_DESC CreateRootSigDesc (RootSigType type);

private:	// メンバ変数
	// 静的配列としてルートパラメータとDescriptorRangeの実体を保持する
	//Standard3D
	D3D12_DESCRIPTOR_RANGE standard3DDescriptorRanges[5] = {};
	D3D12_ROOT_PARAMETER standard3DRootParameters[9] = {};
	D3D12_STATIC_SAMPLER_DESC standard3DStaticSamplers[1] = {};
	//Standard3DInstance
	D3D12_DESCRIPTOR_RANGE standard3DInstanceDescriptorRanges[6] = {};
	D3D12_ROOT_PARAMETER standard3DInstanceRootParameters[9] = {};
	D3D12_STATIC_SAMPLER_DESC standard3DInstanceStaticSamplers[1] = {};
	//SkinningStanderd3D
	D3D12_DESCRIPTOR_RANGE skinningStandard3DDescriptorRanges[6] = {};
	D3D12_ROOT_PARAMETER skinningStandard3DRootParameters[10] = {};
	D3D12_STATIC_SAMPLER_DESC skinningStandard3DStaticSamplers[1] = {};
	//Sprite
	D3D12_DESCRIPTOR_RANGE spriteDescriptorRanges[1] = {};
	D3D12_ROOT_PARAMETER spriteRootParameters[3] = {};
	D3D12_STATIC_SAMPLER_DESC spriteStaticSamplers[1] = {};
	//Particle
	D3D12_DESCRIPTOR_RANGE particleDescriptorRanges[1] = {};
	D3D12_DESCRIPTOR_RANGE textureDescriptorRanges[1] = {};
	D3D12_ROOT_PARAMETER particleRootParameters[3] = {};
	D3D12_STATIC_SAMPLER_DESC particleStaticSamplers[1] = {};
	//Line
	D3D12_DESCRIPTOR_RANGE lineMeshDescriptorRanges[2] = {};
	D3D12_ROOT_PARAMETER lineMeshRootParameters[2] = {};
	//Cube
	D3D12_DESCRIPTOR_RANGE cubeMeshDescriptorRanges[2] = {};
	D3D12_ROOT_PARAMETER cubeMeshRootParameters[2] = {};
	//Skybox
	D3D12_DESCRIPTOR_RANGE skyboxDescriptorRanges[1] = {};
	D3D12_ROOT_PARAMETER skyboxRootParameters[3] = {};
	D3D12_STATIC_SAMPLER_DESC skyboxStaticSamplers[1] = {};
	//***ルートシグネチャの種類を増やしたいときに適宜追加***//

	//RootSigTypeとIDのマップ(逆引き兼キャッシュチェック用)
	std::unordered_map<RootSigType, uint32_t> m_TypeToID;

	//IDとID3D12RootSignatureの実体データのマップ
	std::unordered_map<uint32_t, ComPtr<ID3D12RootSignature>> m_RootSigCache;

	//次に割り当てるID(0は無効なリソースとして扱われてしまう可能性があるので1から連番)
	uint32_t m_NextID = 1;

	//ポインタを借りる
	ID3D12Device* device_ = nullptr;
	ID3D12GraphicsCommandList* commandList_ = nullptr;
};