#pragma once
#include <Windows.h>
#include <Wrl.h>
using namespace Microsoft::WRL;
#include <d3d12.h>
#include <vector>
#include <string>
#include "struct.h"
#include "PSOManager.h"
#include "DxCommon.h"

struct VertexData {
	Vector4 position;
};

class Skybox {
public:
	void Initialize(std::string filePath);

	void Update();

	void Draw();

private:
	
};