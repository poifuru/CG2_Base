#include "CameraOrganizer.h"
#include <imgui.h>

CameraOrganizer::~CameraOrganizer () {
	for (auto const& [key, val] : cameras_) {
		delete val;
	}
	cameras_.clear ();
}

void CameraOrganizer::Initialize (InputManager* inputManager) {
	input_ = inputManager;
	vpMatrix_ = Math::MakeIdentity4x4 ();

	//テスト用 DebugCameraを生成＆登録
	AddCamera ("Debug", CameraType::DebugCamera);

	//初期状態ではデバッグカメラをセット
	SetActiveCamera ("Debug");
	//positionとrotateをセット
	SetPosition ({ 0.0f, 15.0f, -50.0f });
	SetRotate ({ 0.3f, 0.0f, 0.0f });
}

void CameraOrganizer::AddCamera (const std::string& name, CameraType type) {
	//同じ名前のカメラがあったら
	if (cameras_.count (name)) {
		//早期リターン
		return;
	}

	//原点にカメラを生成
	EulerTransform Transform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	CameraComponent* camera = nullptr;

	switch (type) {
	case::CameraType::FixedPointCamera: {
		FixedPointCamera* fixedCam = new FixedPointCamera ();
		camera = fixedCam;
		break;
	}

	case::CameraType::FollowCamera: {
		FollowCamera* followCam = new FollowCamera ();
		camera = followCam;
		break;
	}

	case::CameraType::LookAtCamera: {
		LookAtCamera* lookAtCam = new LookAtCamera (input_);
		camera = lookAtCam;
		break;
	}

	case::CameraType::DebugCamera: {
		DebugCamera* debugCam = new DebugCamera (input_);
		camera = debugCam;
		break;
	}
	}

	//追加するカメラの初期化がうまくいかなかったら
	if (camera == nullptr) {
		//リターン
		return;
	}

	camera->Initialize (Transform);
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
						lastActiveCamera_ = key;
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
	activeCamera_->Update ();
	vpMatrix_ = activeCamera_->GetVPMat ();
}

void CameraOrganizer::ImGui () {
#ifdef USEIMGUI
	ImGui::Begin ("CameraOrganizer");
	ImGui::Separator ();
	// 現在アクティブなカメラの名前を取得
	const char* current_item = activeCameraName_.c_str ();

	// 登録済みのカメラ名を格納するvector
	std::vector<const char*> cameraNames;
	for (const auto& pair : cameras_) {
		// std::stringのポインタを保持
		cameraNames.push_back (pair.first.c_str ());
	}

	// ImGui::Comboを使ってリストからカメラを選択
	if (ImGui::BeginCombo ("##CameraList", current_item)) { // "##CameraList"はラベルを非表示にするテクニック
		for (const char* name : cameraNames) {
			// 現在選択されているアイテムかチェック
			bool is_selected = (current_item == name);

			// Selectableアイテムの描画
			if (ImGui::Selectable (name, is_selected)) {
				// 選択されたらアクティブカメラを切り替える
				SetActiveCamera (name);
			}

			// オートスクロールするために選択されているアイテムにフォーカス
			if (is_selected) {
				ImGui::SetItemDefaultFocus ();
			}
		}
		ImGui::EndCombo ();
	}

	// 現在アクティブなカメラ名を表示（オプション）
	ImGui::Text (("アクティブなカメラ : " + activeCameraName_).c_str ());

	activeCamera_->ImGui ();
	ImGui::End ();
#endif
}

void CameraOrganizer::SetFollowTarget (const std::string& cameraName, const EulerTransform& target) {
	auto it = cameras_.find (cameraName);
	if (it == cameras_.end ()) {
		return;
	}

	CameraComponent* camera = it->second;

	// 2. FollowCamera型にダウンキャストする
	FollowCamera* followCamera = dynamic_cast<FollowCamera*>(camera);

	if (followCamera) {
		// 3. ダウンキャストに成功したら、SetTarget() を呼び出す！
		followCamera->SetTarget (&target);
	}
}

void CameraOrganizer::SetLookAtPosition (const std::string& cameraName, const Vector3& pos) {
	auto it = cameras_.find (cameraName);
	if (it == cameras_.end ()) {
		return;
	}

	CameraComponent* camera = it->second;

	// 2. FollowCamera型にダウンキャストする
	LookAtCamera* lookAtCamera = dynamic_cast<LookAtCamera*>(camera);

	if (lookAtCamera) {
		// 3. ダウンキャストに成功したら、SetTarget() を呼び出す！
		lookAtCamera->SetPosition (pos);
	}
}

void CameraOrganizer::SetLookAtTarget (const std::string& cameraName, const Vector3& targetPos) {
	auto it = cameras_.find (cameraName);
	if (it == cameras_.end ()) {
		return;
	}

	CameraComponent* camera = it->second;

	// 2. FollowCamera型にダウンキャストする
	LookAtCamera* lookAtCamera = dynamic_cast<LookAtCamera*>(camera);

	if (lookAtCamera) {
		// 3. ダウンキャストに成功したら、SetTarget() を呼び出す！
		lookAtCamera->SetTarget (targetPos);
	}
}
