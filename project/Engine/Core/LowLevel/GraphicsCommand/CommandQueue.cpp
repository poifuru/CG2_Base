#include "PCH.h"
#include "CommandQueue.h"
#include "GraphicsCommandCreator.h"

MyEngine::LowLevel::CommandQueue::~CommandQueue() {
	if (fenceEvent_) {
		CloseHandle(fenceEvent_);
	}
}

void MyEngine::LowLevel::CommandQueue::Initialize(ID3D12Device* device) {
	// コマンドキュー生成
	MyEngine::LowLevel::GraphicsCommandCreator::CreateCommandQueue(device, commandQueue_);

	// フェンスを生成
	MyEngine::LowLevel::GraphicsCommandCreator::CreateFence(device, fence_, fenceValue_, fenceEvent_);
}

void MyEngine::LowLevel::CommandQueue::ExecuteCommandList(
	ID3D12GraphicsCommandList* cmdList
) {
	// コマンドリストを閉じて内容を確定させる
	HRESULT hr = cmdList->Close();
	assert(SUCCEEDED(hr));

	// 実際に実行する
	ID3D12CommandList* commandLists[] = { cmdList };
	commandQueue_->ExecuteCommandLists(1, commandLists);
}

void MyEngine::LowLevel::CommandQueue::SignalAndWait() {
	// フェンスのバリューを増やす
	fenceValue_++;

	//　キュー内のタスクが終わり次第fenceの内部カウントをvalueに合わせる
	HRESULT hr = commandQueue_->Signal(fence_.Get(), fenceValue_);
	assert(SUCCEEDED(hr));

	// fence内部のカウントがvalueより小さければ
	if (fence_->GetCompletedValue() < fenceValue_) {
		// 内部カウントがvalueに追いつくまでCPUを待たせる
		hr = fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		assert(SUCCEEDED(hr));
		WaitForSingleObject(fenceEvent_, INFINITE);
	}
}