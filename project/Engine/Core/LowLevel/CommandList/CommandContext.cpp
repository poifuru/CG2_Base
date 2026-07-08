#include "PCH.h"
#include "CommandContext.h"

CommandContext::CommandContext() = default;

CommandContext::~CommandContext() {
	if (fenceEvent_) {
		CloseHandle(fenceEvent_);
	}
}

void CommandContext::Initialize(ID3D12Device* device) {
	// 引数のデバイスが有効かチェック
	assert(device != nullptr);

	HRESULT hr = S_OK;

	// コマンドキューの生成
	D3D12_COMMAND_QUEUE_DESC queueDesc{};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask = 0;

	hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(commandQueue_.GetAddressOf()));
	assert(SUCCEEDED(hr));

	// コマンドアロケーターの生成
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocator_.GetAddressOf()));
	assert(SUCCEEDED(hr));

	// コマンドリストの生成
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(commandList_.GetAddressOf()));
	assert(SUCCEEDED(hr));

	// 生成時はオープン状態なので、最初のフレームの Reset でエラーにならないように一旦閉じておく
	hr = commandList_->Close();
	assert(SUCCEEDED(hr));

	// フェンスの生成とイベント作成
	hr = device->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence_.GetAddressOf()));
	assert(SUCCEEDED(hr));

	fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	assert(fenceEvent_ != nullptr);
}

void CommandContext::Reset() {
	HRESULT hr = commandAllocator_->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
	assert(SUCCEEDED(hr));
}

void CommandContext::Execute() {
	HRESULT hr = commandList_->Close();
	assert(SUCCEEDED(hr));

	ID3D12CommandList* commandLists[] = { commandList_.Get() };
	commandQueue_->ExecuteCommandLists(1, commandLists);
}

void CommandContext::SignalAndWait() {
	fenceValue_++;
	HRESULT hr = commandQueue_->Signal(fence_.Get(), fenceValue_);
	assert(SUCCEEDED(hr));

	if (fence_->GetCompletedValue() < fenceValue_) {
		hr = fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		assert(SUCCEEDED(hr));
		WaitForSingleObject(fenceEvent_, INFINITE);
	}
}

void CommandContext::TransitionBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter) {
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = resource;
	barrier.Transition.StateBefore = stateBefore;
	barrier.Transition.StateAfter = stateAfter;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList_->ResourceBarrier(1, &barrier);
}

void CommandContext::ClearRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, const float color[4]) {
	commandList_->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
}

void CommandContext::ClearDepthBuffer(D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle, float depth) {
	commandList_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}

void CommandContext::SetRenderTargets(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, const D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle) {
	commandList_->OMSetRenderTargets(1, &rtvHandle, FALSE, dsvHandle);
}
