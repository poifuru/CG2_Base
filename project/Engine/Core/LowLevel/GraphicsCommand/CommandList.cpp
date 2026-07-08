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