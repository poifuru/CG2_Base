#pragma once
#include <memory>
#include "CameraOrganizer.h"
#include "InputManager.h"
#include "DxCommon.h"

class BaseScene {
public:		//メンバ関数
	virtual ~BaseScene () = default;

	virtual void Initialize (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) = 0;
	virtual void Update () = 0;
	virtual void Draw () = 0;

protected:	//メンバ変数
	//次に行きたいシーンを持たせる
	BaseScene* nextScene_ = nullptr;

	//ポインタを借りる
	CameraOrganizer* camera_ = nullptr;
	InputManager* input_ = nullptr;
};
