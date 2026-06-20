#pragma once
#include "BaseScene.h"
#include <memory>
#include "Model.h"
#include "LightManager.h"

class PlayScene : public BaseScene {
public:
	PlayScene() = default;
	~PlayScene() override = default;

	void Initialize() override;
	void Update(CameraData* cameraData) override;
	void Draw(class RenderSystem* renderSystem) override;

private:
	std::unique_ptr<Model> model_ = nullptr;
	std::unique_ptr<LightManager> lightManager_ = nullptr;
};
