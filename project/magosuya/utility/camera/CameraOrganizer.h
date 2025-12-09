#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "CameraComponent.h"
#include "DebugCamera.h"
#include "FixedPointCamera.h"
#include "FollowCamera.h"
#include "LookAtCamera.h"
#include "MathFunction.h"
#include "InputManager.h"

enum class CameraType {
	FixedPontCamera,
	FollowCamera,
	LookAtCamera,
	DebugCamera,
};

class CameraOrganizer {
public:
	static CameraOrganizer* GetInstance () {
		//初めて呼び出されたときに一回だけ初期化
		static CameraOrganizer instance;
		return &instance;
	}
	~CameraOrganizer ();

	//初期化関数
	void Initialize (InputManager* inputManager);

	//カメラを登録する関数
	void AddCamera (const std::string& name, CameraType type);

	//アクティブなカメラを切り替える
	void SetActiveCamera (const std::string& cameraName);

	//アクティブカメラの更新処理
	void Update ();

	//ImGui
	void ImGui ();

	//追従カメラの設定をいじるためにコンテナからカメラを取得
	void SetFollowTarget (const std::string& cameraName, const Transform& target);

	//LookAtカメラの設定をいじるためにコンテナからカメラを取得
	void SetLookAtTarget (const std::string& cameraName, const Vector3& targetPos);

	//描画用のvp行列取得関数
	Matrix4x4* GetVPMatrix () { return &vpMatrix_; }

	//位置と回転をいじれるように
	void SetPosition (const Vector3& position) { activeCamera_->SetTranslate (position); }
	void SetRotate (const Vector3& rotate) { activeCamera_->SetRotate (rotate); }

private:
	//コンストラクタを禁止
	CameraOrganizer () = default;
	// コピーコンストラクタと代入演算子を禁止
	CameraOrganizer (const CameraOrganizer&) = delete;
	CameraOrganizer& operator=(const CameraOrganizer&) = delete;
	CameraOrganizer (CameraOrganizer&&) = delete;
	CameraOrganizer& operator=(CameraOrganizer&&) = delete;

private:
	//カメラを収納するコンテナ
	std::unordered_map<std::string, CameraComponent*> cameras_;

	//アクティブ状態のカメラ
	CameraComponent* activeCamera_ = nullptr;

	//vp行列
	Matrix4x4 vpMatrix_ = {};

	//最後にアクティブだったカメラの名前
	std::string lastAcriveCamera_;

	//現在アクティブなカメラの名前
	std::string activeCameraName_;

	//ポインタを借りる
	InputManager* input_ = nullptr;
};