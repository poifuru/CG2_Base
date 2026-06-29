#include "PCH.h"
#include "VirtualFollowCamera.h"
#include "GameObject.h"
#include "MathFunction.h"
#include "BaseScene.h" // SceneContextやGameObject検索用

void VirtualFollowCamera::Update() {
	// ターゲットが未設定で、ターゲット名がある場合はシーンから探す
	if (!target_ && !targetName_.empty() && gameObject_) {
		// ※シーン全体のオブジェクトリストから名前で検索する
		// target_ = gameObject_->GetContext()->scene->FindObject(targetName_);
	}

	if (target_ && gameObject_) {
		// ターゲット位置 + オフセット = 目標座標
		Vector3 targetPos = Math::Add(target_->GetTransform().translate, offset_);

		// 自身の座標を滑らかに補間して追従させる
		auto& myTransform = gameObject_->GetTransform();

		myTransform.translate = Math::Lerp(myTransform.translate, targetPos, 1.0f - delay_);
		// 常にターゲットの方を向く（LookAt回転）の計算
		Vector3 targetCenter = target_->GetTransform().translate; // ターゲットの位置（注視点）
		Vector3 direction = Math::Subtract(targetCenter, myTransform.translate); // カメラから見たターゲットへの方向
		
		if(Math::Length(direction) > 0.001f) {
			// ヨー回転（Y軸まわり）の計算
			myTransform.rotate.y = std::atan2(direction.x, direction.z);

			// ピッチ回転（X軸まわり）の計算
			float xzLength = std::sqrt(direction.x * direction.x + direction.z * direction.z);
			myTransform.rotate.x = std::atan2(-direction.y, xzLength);

			// ロール回転（Z軸まわり）は0にリセット
			myTransform.rotate.z = 0.0f;
		}
	}
}

void VirtualFollowCamera::ImGui() {
	VirtualCameraComponent::ImGui(); // 親のImGui（優先度・FOV）を描画

	ImGui::Separator();
	char nameBuf[256];
	strcpy_s(nameBuf, targetName_.c_str());
	if (ImGui::InputText("Target Object Name", nameBuf, sizeof(nameBuf))) {
		targetName_ = nameBuf;
		target_ = nullptr; // ターゲットを再検索させる
	}
	ImGui::DragFloat3("Offset", &offset_.x, 0.1f);
	ImGui::SliderFloat("Delay", &delay_, 0.0f, 1.0f);
}

void VirtualFollowCamera::Serialize(json& j) const {
	VirtualCameraComponent::Serialize(j);
	j["type"] = "VirtualFollowCamera";
	j["targetName"] = targetName_;
	j["offset"] = { offset_.x, offset_.y, offset_.z };
	j["delay"] = delay_;
}

void VirtualFollowCamera::Deserialize(const json& j) {
	VirtualCameraComponent::Deserialize(j);
	if (j.contains("targetName")) targetName_ = j["targetName"];
	if (j.contains("offset")) {
		offset_ = { j["offset"][0], j["offset"][1], j["offset"][2] };
	}
	if (j.contains("delay")) delay_ = j["delay"];
}

void VirtualFollowCamera::ResolveTarget(const std::vector<std::unique_ptr<GameObject>>& gameObjects) {
	if (targetName_.empty()) return;
	for (const auto& obj : gameObjects) {
		if (obj->GetName() == targetName_) {
			target_ = obj.get();
			return;
		}
	}
}