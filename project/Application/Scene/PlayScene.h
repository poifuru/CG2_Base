#pragma once
#include "BaseScene.h"
#include <memory>
#include "Model.h"

class PlayScene : public BaseScene {
public:
	PlayScene() = default;
	~PlayScene() override = default;

	void Initialize() override;
	void Update(CameraData* cameraData) override;
	void Draw(class RenderSystem* renderSystem) override;

private:
	std::unique_ptr<Model> triangleModel_ = nullptr;
	uint32_t vsID_ = 0;
	uint32_t psID_ = 0;
	uint32_t textureIndex_ = 0;
};
