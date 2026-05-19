#pragma once
#include "BaseObject3d.h"
#include "MeshData.h"

class Model : public BaseObject3d {
public:
	Model(DxCommon* dxCommon, LightManager* lightManager);
	~Model() override;

	void Initialize(const std::string& modelID);

	void Update(CameraData* cameraData) override;

	void Draw() override;

private:
	// モデルのデータ
	MeshData* modelData_ = nullptr;
};