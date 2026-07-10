#include "PCH.h"
#include "CommandList.h"
#include "GraphicsCommandCreator.h"

void MyEngine::LowLevel::CommandList::Initialize(Microsoft::WRL::ComPtr<ID3D12Device> device) {
	// コマンドアロケータを生成
	MyEngine::LowLevel::GraphicsCommandCreator::CreateCommandAllocator(device, commandAllocator_);

	// コマンドリストを生成
	MyEngine::LowLevel::GraphicsCommandCreator::CreateCommandList(device, commandAllocator_, commandList_);
}

void MyEngine::LowLevel::CommandList::Reset() {
	// コマンドアロケータのリセット
	HRESULT hr = commandAllocator_->Reset();
	assert(SUCCEEDED(hr));

	// コマンドリストのリセット
	hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
	assert(SUCCEEDED(hr));
}

void MyEngine::LowLevel::CommandList::ClearRenderTarget(
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle,
	const float color[4]
) {
	commandList_->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
}

void MyEngine::LowLevel::CommandList::SetRenderTargets(
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle,
	const D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle
) {
	commandList_->OMSetRenderTargets(1, &rtvHandle, FALSE, dsvHandle);
}

void MyEngine::LowLevel::CommandList::ClearDepthBuffer(
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle,
	float depth
) {
	commandList_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}