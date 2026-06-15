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
	// 複数生成するオブジェクトの名前で分岐
	
	// 未知のタイプの場合はnullptrを返す
	return nullptr;
}
