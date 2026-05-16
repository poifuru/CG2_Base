#pragma once
#include <d3d12.h>

struct DescriptorHandle {
	D3D12_CPU_DESCRIPTOR_HANDLE cpu{ 0 };
	D3D12_GPU_DESCRIPTOR_HANDLE gpu{ 0 };

	// デフォルトコンストラクタ
	DescriptorHandle() = default;

	// CPUハンドルだけ、あるいは両方を代入して初期化できるコンストラクタ
	DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE c, D3D12_GPU_DESCRIPTOR_HANDLE g = { 0 }){
		cpu = c;
		gpu = g;
	}

	// ハンドルが有効（初期化済み）かどうかを判定する便利メソッド
	bool IsValid() const { return cpu.ptr != 0; }

	// GPU側が利用可能（シェーダーにバインドできるか）判定
	bool HasGpuHandle() const { return gpu.ptr != 0; }
};