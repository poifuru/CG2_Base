#include "PCH.h"
#include "GraphicsCommandCreator.h"

void MyEngine::LowLevel::GraphicsCommandCreator::CreateCommandQueue(
	Microsoft::WRL::ComPtr<ID3D12Device> device,
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>& commandQueue
) {
	// 引数のデバイスが有効かチェック
	assert(device != nullptr);

	HRESULT hr = S_OK;

	// コマンドキューの生成
	D3D12_COMMAND_QUEUE_DESC queueDesc{};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask = 0;

	hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(commandQueue.GetAddressOf()));
	assert(SUCCEEDED(hr));
}

void MyEngine::LowLevel::GraphicsCommandCreator::CreateCommandAllocator(
	Microsoft::WRL::ComPtr<ID3D12Device> device,
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& commandAllocator
) {
	// 引数のデバイスが有効かチェック
	assert(device != nullptr);

	HRESULT hr = S_OK;

	// コマンドアロケーターの生成
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocator.GetAddressOf()));
	assert(SUCCEEDED(hr));
}

void MyEngine::LowLevel::GraphicsCommandCreator::CreateCommandList(
	Microsoft::WRL::ComPtr<ID3D12Device> device,
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& commandAllocator,
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList
) {
	// 引数のデバイスが有効かチェック
	assert(device != nullptr);

	HRESULT hr = S_OK;

	// コマンドリストの生成
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(commandList.GetAddressOf()));
	assert(SUCCEEDED(hr));

	// 生成時はオープン状態なので、最初のフレームの Reset でエラーにならないように一旦閉じておく
	hr = commandList->Close();
	assert(SUCCEEDED(hr));
}

void MyEngine::LowLevel::GraphicsCommandCreator::CreateFence(
	Microsoft::WRL::ComPtr<ID3D12Device> device,
	Microsoft::WRL::ComPtr<ID3D12Fence>& fence,
	uint64_t& fenceValue,
	HANDLE& fenceEvent
) {
	// 引数のデバイスが有効かチェック
	assert(device != nullptr);

	HRESULT hr = S_OK;

	// フェンスの生成とイベント作成
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()));
	assert(SUCCEEDED(hr));

	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	assert(fenceEvent != nullptr);
}