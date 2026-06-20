#pragma once
#include <string>
#include <cstdint>
#include <memory>

class Model;
class RenderSystem;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12CommandQueue;
class GraphicsDevice;
class DescriptorHeapManager;
class ShaderManager;
class RenderTexture;

class IEngine {
public:
	virtual ~IEngine() = default;

	virtual void Initialize() = 0;
	virtual bool ProcessMessage() = 0;
	virtual void BeginFrame() = 0;
	virtual void EndFrame() = 0;

	// レンダーシステムを取得するインターフェース
	virtual RenderSystem* GetRenderSystem() = 0;

	// 低レイヤーアクセッサ群
	virtual ID3D12Device* GetDevice() = 0;
	virtual GraphicsDevice* GetGraphicsDevice() = 0;
	virtual ID3D12GraphicsCommandList* GetCommandList() = 0;
	virtual ID3D12CommandQueue* GetCommandQueue() = 0;
	virtual DescriptorHeapManager* GetDescriptorHeapManager() = 0;
	virtual ShaderManager& GetShaderManager() = 0;
	virtual RenderTexture* GetRenderTexture() = 0;

	// コマンドリスト制御
	virtual void ResetCommandList() = 0;
	virtual void ExecuteCommandList() = 0;
};