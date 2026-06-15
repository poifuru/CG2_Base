#include "GameObjectFactory.h"
// オブジェクトのインクルード
#include "Player.h"

std::unique_ptr<GameObject> GameObjectFactory::Create(
	const std::string& typeName,
	DxCommon* dxCommon,
	LightManager* light,
	InputManager* input,
	CameraOrganizer* camera
) {
	if(typeName == "Player") {
		auto obj = std::make_unique<Player>(dxCommon, camera, input);
		obj->Initialize();

		return obj;
	}

	// 未知のタイプの場合はnullptrを返す
	return nullptr;
}
