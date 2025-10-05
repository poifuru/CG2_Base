#include "Triangle.h"
#include "../../engine/utility/function.h"
#include "../../engine/utility/Math.h"

void Triangle::Initialize (ID3D12Device* device) {
	//***バッファ作成とマッピングを済ませる***//
     // 頂点バッファ作成
    vertexBuffer_ = CreateBufferResource (device, sizeof (VertexData) * 3);
    vertexBuffer_->Map (0, nullptr, reinterpret_cast<void**>(&vertexData_));
    vbView_.SizeInBytes = sizeof (VertexData) * 3;
    vbView_.StrideInBytes = sizeof (VertexData);
    //vertexData_[0]->

    // 行列バッファ作成
    matrixBuffer_ = CreateBufferResource (device, sizeof (TransformationMatrix));
    matrixBuffer_->Map (0, nullptr, reinterpret_cast<void**>(&matrixData_));
    *matrixData_ = MakeIdentity4x4 ();

    // マテリアル作成
    materialBuffer_ = CreateBufferResource (device, sizeof (Material));
    materialBuffer_->Map (0, nullptr, reinterpret_cast<void**>(&materialData_));
    materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };	//初期カラーは白
    materialData_->enableLighting = false;
    materialData_->uvTranform = MakeIdentity4x4 ();

    //// ライト作成
    //lightBuffer_ = CreateBufferResource (device, sizeof (DirectionalLight));
    //lightBuffer_->Map (0, nullptr, reinterpret_cast<void**>(&lightData_));
}

void Triangle::Update (ShapeData* shapeData) {
    
}

void Triangle::Command (ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE* textureSrvHandleGPU) {
    //どんな形状で描画するのか
    commandList->IASetPrimitiveTopology (D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    //頂点バッファをセットする
    commandList->IASetVertexBuffers (0, 1, &vbView_);	//VBVを設定
    //定数バッファのルートパラメータを設定する	
    commandList->SetGraphicsRootConstantBufferView (0, matrixBuffer_->GetGPUVirtualAddress ());
    commandList->SetGraphicsRootConstantBufferView (1, materialBuffer_->GetGPUVirtualAddress ());
    //テクスチャのSRVを設定
    commandList->SetGraphicsRootDescriptorTable (2, *textureSrvHandleGPU);
    ////ライティングの設定
    //commandList->SetGraphicsRootConstantBufferView (3, lightBuffer_->GetGPUVirtualAddress ());
    //実際に描画する
    commandList->DrawInstanced (3, 1, 0, 0);
}