#include "PCH.h"
#include "CommandContext.h"

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
