#pragma once
#include <memory>
#include <string>
#include "GameObject.h"

class DxCommon;
class LightManager;
class InputManager;
class CameraOrganizer;

class GameObjectFactory {
public:
	static std::unique_ptr<GameObject> Create(
		const std::string& typeName,
		DxCommon* dxCommon,
		LightManager* light,
		InputManager* input,
		CameraOrganizer* camera
	);
};