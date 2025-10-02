#pragma once
#include "../../header/struct.h"
#include "../../externals/DirectXTex/d3dx12.h"

class Shape {
public:
	//とりあえず出せるように
	static void DrawTriangle (ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ShapeData* shapeData);
};

