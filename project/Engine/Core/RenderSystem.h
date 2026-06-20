#pragma once
#include <vector>
#include <d3d12.h>
#include "RenderCommand.h"
#include "ConstantBuffer.h"
#include "struct.h"

class PSOManager;
class DescriptorHeapManager;
class ShaderManager;
class LightManager;

struct CameraForGPU {
	Vector3 worldPosition;
	float padding; // 16バイトアライメント用のパディング
};

class RenderSystem {
public:
	RenderSystem() = default;
	~RenderSystem() = default;

	// リソース初期化用
	void Initialize(ID3D12Device* device);

	// コマンドの積み込み
	void PushCommand(const RenderCommand& command);

	// カメラ位置の設定・更新用
	void SetCameraPosition(const Vector3& cameraPos);

	// アクティブなライトマネージャーの登録用
	void SetLightManager(LightManager* lightManager);

	void ExecuteCommands(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		ID3D12RootSignature* commonRootSignature,
		DescriptorHeapManager& heapManager,
		PSOManager& psoManager,
		const ShaderManager& shaderManager,
		const InputLayoutManager& inputLayoutManager,
		const BlendModeManager& blendModeManager
	);

	void ClearCommands();

public:
	// コピー・移動禁止
	RenderSystem(const RenderSystem&) = delete;
	RenderSystem& operator=(const RenderSystem&) = delete;
	RenderSystem(RenderSystem&&) = delete;
	RenderSystem& operator=(RenderSystem&&) = delete;

private:
	std::vector<RenderCommand> commandQueue_;

	// カメラ用の定数バッファ
	ConstantBuffer<CameraForGPU> cameraBuffer_;

	// アクティブなライトマネージャーへのポインタ
	LightManager* activeLightManager_ = nullptr;
};