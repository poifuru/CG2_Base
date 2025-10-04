#include "Shape.h"
#include "../../engine/utility/function.h"

void Shape::Initialize (ID3D12Device* device, ID3D12GraphicsCommandList* commandList) {
	device_ = device;
	commandList_ = commandList;
	triangle_.Initialize (device);
}

void Shape::DrawTriangle (ShapeData* shapeData, D3D12_GPU_DESCRIPTOR_HANDLE* textureSrvHandleGPU) {
	triangle_.Update (shapeData);
	triangle_.Command (commandList_.Get (), textureSrvHandleGPU);
}