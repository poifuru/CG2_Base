#include "PCH.h"
#include "ReticleComponent.h"
#include "GameObject.h"
#include "MathFunction.h"
#include "CameraOrganizer.h"
#include "BaseCamera.h"
#include "BaseScene.h"
#include "BirdEnemyComponent.h"
#include "FishEnemyComponent.h"
#include "MeshRendererComponent.h"

void ReticleComponent::Initialize() {
	// すでに初期化済み（ロード済み）なら、デフォルト値での上書きをスキップする
	if (isInitialized_) return;
	isInitialized_ = true;

	offsetZ_ = 20.0f;
}

void ReticleComponent::Update() {
	if (!gameObject_ && !isDebugMode_) return;

	// シングルトンのカメラオーガナイザーから現在のアクティブカメラの情報を取る
	CameraOrganizer* cameraOrganizer = CameraOrganizer::GetInstance();
	CameraData& cameraData = cameraOrganizer->GetCameraData();

	// カメラのワールド行列から「位置」と「前方ベクトル」を抽出する
	Vector3 camPos = { cameraData.world.m[3][0], cameraData.world.m[3][1], cameraData.world.m[3][2] };
	Vector3 camForward = { cameraData.world.m[2][0], cameraData.world.m[2][1], cameraData.world.m[2][2] };
	
	// カメラの正面方向に offsetZ_ 分だけ進んだワールド座標を計算
	Vector3 targetPos = Math::Add(camPos, Math::Multiply(offsetZ_, camForward));

	// レティクルの位置を更新
	gameObject_->GetTransform().translate = targetPos;

	// レティクルの回転もカメラと同期させる（ビルボード）
	gameObject_->GetTransform().rotate = cameraData.transform.rotate;

	// 敵オブジェクトを検索して、一番近い敵をロックオンする
	lockOnTarget_ = nullptr;
	float closestDist = 3.0f; // ロックオンできるレティクルからの距離（m）
	auto* context = gameObject_->GetContext();
	if (context && context->gameObjects) {
		for (const auto& obj : *(context->gameObjects)) {
			// 鳥か魚のコンポーネントを持っているか、名前がEnemyのものを敵とみなす
			if (obj->GetComponent<BirdEnemyComponent>() || 
				obj->GetComponent<FishEnemyComponent>() || 
				obj->GetName() == "Enemy") 
			{
				float dist = Math::Length(Math::Subtract(obj->GetTransform().translate, targetPos));
				if (dist < closestDist) {
					closestDist = dist;
					lockOnTarget_ = obj.get();
				}
			}
		}
	}
	// ロックオン中ならレティクルの色を赤にする（マテリアルカラーの変更）
	if (auto* renderer = gameObject_->GetComponent<MeshRendererComponent>()) {
		if (lockOnTarget_) {
			// ロックオン中：赤
			renderer->SetColor({ 1.0f, 0.0f, 0.0f, 0.5f });
		} else {
			// 通常時：黒（元の色）
			renderer->SetColor({ 0.0f, 0.0f, 0.0f, 0.5f });
		}
	}
}

void ReticleComponent::ImGui() {
	ImGui::DragFloat("offsetZ", &offsetZ_, 0.01f);
}

void ReticleComponent::Serialize(json& j) const {
	j["type"] = "ReticleComponent";
	j["offsetZ"] = offsetZ_;
}

void ReticleComponent::Deserialize(const json& j) {
	isInitialized_ = true;
	if (j.contains("offsetZ")) offsetZ_ = j["offsetZ"];
}