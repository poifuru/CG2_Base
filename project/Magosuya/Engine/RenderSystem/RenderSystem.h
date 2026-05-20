#pragma once
#include <vector>
#include "RenderCommand.h"

class DxCommon;
class LightManager;

class RenderSystem {
public:
	static RenderSystem* GetInstance() {
		static RenderSystem instance;
		return &instance;
	}

	void Initialize(DxCommon* dxCommon, LightManager* lightManager);

	void PushCommand(const RenderCommand& command);

	void PushInstanceCommand(const InstanceRenderCommand& command);

	void ExecuteCommands(D3D12_GPU_VIRTUAL_ADDRESS cameraCBVAddress);

	void ClearCommands();

public:
	//コンストラクタを禁止
	//RenderSystem();
	~RenderSystem() = default; // unique_ptrのデストラクタ解決のため明示的に宣言
	// コピーコンストラクタと代入演算子を禁止
	RenderSystem(const RenderSystem&) = delete;
	RenderSystem& operator=(const RenderSystem&) = delete;
	RenderSystem(RenderSystem&&) = delete;
	RenderSystem& operator=(RenderSystem&&) = delete;

private:
	RenderSystem() = default;

	// コマンドをため込む
	std::vector<RenderCommand> commandQueue_;
	std::vector<InstanceRenderCommand> instanceCommandQueue_;

	DxCommon* dxCommon_ = nullptr;
	ID3D12GraphicsCommandList* commandList_ = nullptr;
	LightManager* lightManager_ = nullptr;
};