#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "CameraComponent.h"
#include "DebugCamera.h"
#include "FixedPointCamera.h"
#include "FollowCamera.h"
#include "MathFunction.h"
#include "InputManager.h"

class CameraOrganizer {
public:
	//カメラを登録する関数
	void AddCamera (const std::string& name, CameraComponent* camera);

	//アクティブなカメラを切り替える
	void SetActiveCamera (const std::string& cameraName);

	//アクティブカメラの更新処理
	void Update ();

	//描画用のvp行列取得関数
	const Matrix4x4& GetVPMatrix () { return vpMatrix_; }

private:
	//カメラを収納するコンテナ
	std::unordered_map<std::string, CameraComponent*> cameras_;

	//アクティブ状態のカメラ
	CameraData* activeCamera_ = nullptr;

	//vp行列
	Matrix4x4 vpMatrix_ = {};

	//ポインタを借りる
	InputManager* inputManager_ = nullptr;
};