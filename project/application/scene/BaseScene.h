#pragma once

class TextureManager;
class ModelFactory;
class ShaderManager;
class ModelManager;
struct CameraData;
class GameObject;

namespace MyEngine::LowLevel {
	class GraphicsDevice;
	class DescriptorHeapManager;
}

namespace MyEngine::Rendering {
	class RenderSystem;
}

// シーンで必要になる高レベルマネージャーや低レイヤー参照のポインタを束ねた薄い構造体
struct SceneContext {
	TextureManager* textureManager = nullptr;
	ModelFactory* modelFactory = nullptr;
	ShaderManager* shaderManager = nullptr;
	ModelManager* modelManager = nullptr;
	MyEngine::LowLevel::GraphicsDevice* graphicsDevice = nullptr;
	MyEngine::LowLevel::DescriptorHeapManager* heapManager = nullptr;

	// 動的追加のためにオブジェクトリストのポインタを載せる
	std::vector<std::unique_ptr<GameObject>>* gameObjects = nullptr;

	// 現在シーンに存在する生存オブジェクトリストへのポインタ
	std::vector<std::unique_ptr<GameObject>>* activeGameObjects = nullptr;
};

class BaseScene {
public:
	virtual ~BaseScene () = default;

	// コンテキストの注入
	void SetContext(SceneContext* context) {
		context_ = context;
	}

	void SetRenderSystem(MyEngine::Rendering::RenderSystem* renderSystem) { renderSys_ = renderSystem; }

	virtual void Initialize () = 0;
	virtual void Update (CameraData* cameraData) = 0;
	virtual void Draw (MyEngine::Rendering::RenderSystem* renderSystem) = 0;
	virtual void DrawUI () {}

protected:
	// 借りてきたポインタ群
	SceneContext* context_ = nullptr;
	MyEngine::Rendering::RenderSystem* renderSys_ = nullptr;
};
