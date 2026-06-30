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
#include "ColliderComponent.h"

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

	// 角度（内積）で判定する 1.0に近いほど画面中央。0.985f は画面中心から約10度以内の範囲
	float maxCos = lockOnAngleCos_; 

	auto* context = gameObject_->GetContext();
	if (context && context->activeGameObjects) {
		for (const auto& obj : *(context->activeGameObjects)) {
			// プレイヤー自身、弾、レティクル、カメラ以外の「コライダー付きオブジェクト」をすべて敵とみなす！
			if (obj->GetName() != "Player" && 
				obj->GetName() != "PlayerBullet" && 
				obj->GetName() != "Reticle" && 
				obj->GetName() != "FollowCamera" &&
				obj->GetComponent<ColliderComponent>() != nullptr) {

				Vector3 toEnemy = Math::Subtract(obj->GetTransform().translate, camPos);
				float distToEnemy = Math::Length(toEnemy);
				Vector3 dirToEnemy = Math::Normalize(toEnemy);
				float cosAngle = Math::Dot(dirToEnemy, camForward);

				// ★【デバッグ用ログ】敵ごとの距離と角度（内積）を全部出力する！
				char debugMsg[256];
				sprintf_s(debugMsg, "[LockOn-Check] Target:%s | Dist:%.2f | Cos:%.4f (TargetCos:%.3f)\n", 
						  obj->GetName().c_str(), distToEnemy, cosAngle, maxCos);
				OutputDebugStringA(debugMsg);
				if (distToEnemy > 1.0f && distToEnemy < 150.0f) {
					if (cosAngle > maxCos) {
						maxCos = cosAngle;
						lockOnTarget_ = obj.get();
					}
				}
			}
		}
	}

	// ★【デバッグ用追加】ロックオンに成功しているか出力する
	if (lockOnTarget_) {
		char debugMsg[256];
		sprintf_s(debugMsg, "[LockOn] Target Found: %s (Cos: %.4f)\n", 
				  lockOnTarget_->GetName().c_str(), maxCos);
		OutputDebugStringA(debugMsg);
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
	ImGui::SliderFloat("LockOn Sensitivity", &lockOnAngleCos_, 0.900f, 1.000f, "%.4f");
}

void ReticleComponent::Serialize(json& j) const {
	j["type"] = "ReticleComponent";
	j["offsetZ"] = offsetZ_;
	j["lockOnAngleCos"] = lockOnAngleCos_;
}

void ReticleComponent::Deserialize(const json& j) {
	isInitialized_ = true;
	if (j.contains("offsetZ")) offsetZ_ = j["offsetZ"];
	if (j.contains("lockOnAngleCos")) lockOnAngleCos_ = j["lockOnAngleCos"];
}