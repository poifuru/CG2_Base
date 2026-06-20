#pragma once
#include <memory>

class TextureManager;
class ModelFactory;
class ShaderManager;
class ModelManager;
struct CameraData;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
class DescriptorHeapManager;

// シーンで必要になる高レベルマネージャーや低レイヤー参照のポインタを束ねた薄い構造体
struct SceneContext {
	TextureManager* textureManager = nullptr;
	ModelFactory* modelFactory = nullptr;
	ShaderManager* shaderManager = nullptr;
	ModelManager* modelManager = nullptr;
	ID3D12Device* device = nullptr;
	ID3D12GraphicsCommandList* cmdList = nullptr;
	DescriptorHeapManager* heapManager = nullptr;
};

class BaseScene {
public:
	virtual ~BaseScene () = default;

	// コンテキストの注入
	void SetContext(SceneContext* context) {
		context_ = context;
	}

	virtual void Initialize () = 0;
	virtual void Update (CameraData* cameraData) = 0;
	virtual void Draw (class RenderSystem* renderSystem) = 0;
	virtual void DrawUI () {}

protected:
	// 借りてきたポインタ群
	SceneContext* context_ = nullptr;
};
