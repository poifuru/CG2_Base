#include "CameraOrganizer.h"
#include <imgui.h>

CameraOrganizer::~CameraOrganizer () {
	for (auto const& [key, val] : cameras_) {
		delete val;
	}
	cameras_.clear ();
}

void CameraOrganizer::Initialize () {
	inputManager_ = InputManager::GetInstance ();
	vpMatrix_ = Math::MakeIdentity4x4 ();

	//テスト用 DebugCameraを生成＆登録
	Transform transform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -30.0f} };
	AddCamera ("Debug", CameraType::DebugCamera, transform);

	//初期状態ではデバッグカメラをセット
	SetActiveCamera ("Debug");
}

void CameraOrganizer::AddCamera (const std::string& name, CameraType type, const Transform& transform) {
	//同じ名前のカメラがあったら
	if (cameras_.count (name)) {
		//早期リターン
		return;
	}

	CameraComponent* camera = nullptr;

	switch (type) {
	case::CameraType::FixedPontCamera: {
		FixedPointCamera* fixedCam = new FixedPointCamera ();
		camera = fixedCam;
		break;
	}

	case::CameraType::FollowCamera: {
		FollowCamera* followCam = new FollowCamera ();
		camera = followCam;
		break;
	}

	case::CameraType::DebugCamera: {
		DebugCamera* debugCam = new DebugCamera ();
		camera = debugCam;
		break;
	}
	}

	//追加するカメラの初期化がうまくいかなかったら
	if (camera == nullptr) {
		//リターン
		return;
	}

	camera->Initialize (transform);
	cameras_[name] = camera;
}

void CameraOrganizer::SetActiveCamera (const std::string& cameraName) {
	auto it = cameras_.find (cameraName);
	if (it != cameras_.end ()) {
		if (activeCamera_ && activeCamera_ != it->second) {
			if (dynamic_cast<DebugCamera*>(activeCamera_) == nullptr) {
				//アクティブなカメラをコンテナから逆引き
				for (auto const& [key, val] : cameras_) {
					//ポインタが一致しているか
					if (val == activeCamera_) {
						//一致する名前を保存
						lastAcriveCamera_ = key;
						break;
					}
				}
			}
		}
	}
	activeCamera_ = it->second;
	activeCameraName_ = cameraName;
}

void CameraOrganizer::Update () {
	//デバッグカメラ切り替え
	if (inputManager_->GetRawInput ()->Trigger (VK_TAB)) {
		//現在のカメラがデバッグカメラであるかをチェック
		if (activeCamera_ && dynamic_cast<DebugCamera*>(activeCamera_)) {
			//デバッグカメラなら前回使用していたカメラをセット
			SetActiveCamera (lastAcriveCamera_);
			activeCameraName_ = lastAcriveCamera_;
		}
		else {
			//デバッグカメラでないなら切り替え
			SetActiveCamera ("Debug");
		}
	}
	activeCamera_->Update ();
	vpMatrix_ = activeCamera_->GetVPMat ();
}

void CameraOrganizer::ImGui () {
	ImGui::Begin ("CameraOrganizer");
	ImGui::Text (("アクティブなカメラ : " + activeCameraName_).c_str ());
	ImGui::Separator ();
	activeCamera_->ImGui ();
	ImGui::End ();
}