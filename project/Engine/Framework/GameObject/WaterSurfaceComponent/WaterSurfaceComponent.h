#pragma once
#include "MeshRendererComponent.h"

class WaterSurfaceComponent : public MeshRendererComponent {
public:
	void Initialize();

	void Update();

	void Serialize(json& j) const override;
	void Deserialize(const json& j) override;

	const char* GetName() const override { return "Water Surface"; }

private:
	float time_ = 0.0f;
};