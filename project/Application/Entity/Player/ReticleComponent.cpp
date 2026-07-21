#include "PCH.h"
#include "ReticleComponent.h"
#include "GameObject.h"
#include "MathFunction.h"
#include "CameraOrganizer.h"
#include "BaseCamera.h"
#include "BaseScene.h"
#include "MeshRendererComponent.h"
#include "ColliderComponent.h"
#include "VirtualFollowCamera.h"
#include "RenderingModel.h"
#include "PlayerComponent.h"

void ReticleComponent::Initialize() {
	// すでに初期化済み（ロード済み）なら、デフォルト値での上書きをスキップする
	if (isInitialized_) return;
	isInitialized_ = true;

	offsetZ_ = 20.0f;
}

void ReticleComponent::Update() {
	if (!gameObject_ && !isDebugMode_) return;

	auto* context = gameObject_->GetContext();

	// 固定カメラが最優先（優先度100以上）になっているかチェック
	bool isFixedCameraActive = false;
	{
		if (context && context->activeGameObjects) {
			for (const auto& obj : *(context->activeGameObjects)) {
				if (obj->GetName() == "FixedPointCamera") {
					if (auto* followCam = obj->GetComponent<VirtualFollowCamera>()) {
						if (followCam->GetPriority() >= 100) {
							isFixedCameraActive = true;
							break;
						}
					}
				}
			}
		}
	}

	if (isFixedCameraActive) {
		lockOnTarget_ = nullptr;
		// メッシュレンダラーを取得して透明にし非表示化
		if (auto* renderer = gameObject_->GetComponent<MeshRendererComponent>()) {
			renderer->SetColor({ 0.0f, 0.0f, 0.0f, 0.0f });
		}
		return; // 位置更新やロックオン判定をスキップ
	}

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

	// プレイヤーの船の座標を取得（距離判定を船基準にするため）
	Vector3 playerPos = { 0.0f, 0.0f, 0.0f };
	bool foundPlayer = false;
	if (context && context->activeGameObjects) {
		for (const auto& obj : *(context->activeGameObjects)) {
			if (obj->GetName() == "Player" || obj->GetComponent<PlayerComponent>() != nullptr) {
				playerPos = obj->GetTransform().translate;
				foundPlayer = true;
				break;
			}
		}
	}

	// 角度（内積）で判定する 1.0に近いほど画面中央。0.990f は画面中心から約8度以内の範囲
	float maxCos = 0.990f; 

	if (context && context->activeGameObjects) {
		for (const auto& obj : *(context->activeGameObjects)) {
			// プレイヤー自身、弾、レティクル、カメラ以外の「コライダー付きオブジェクト」をすべて敵とみなす！
			if (obj->GetName() != "Player" && 
				obj->GetName() != "PlayerBullet" && 
				obj->GetName() != "Reticle" && 
				obj->GetName() != "FollowCamera" &&
				obj->GetComponent<ColliderComponent>() != nullptr) {

				// 距離判定はカメラ（+15mオフセット問題）ではなく「プレイヤーの船」からの正確な距離で計算する！
				Vector3 toEnemyFromPlayer = Math::Subtract(obj->GetTransform().translate, foundPlayer ? playerPos : camPos);
				float distToEnemy = Math::Length(toEnemyFromPlayer);

				// 向き（画面中央との一致度）はカメラの視線で計算
				Vector3 toEnemyFromCam = Math::Subtract(obj->GetTransform().translate, camPos);
				Vector3 dirToEnemy = Math::Normalize(toEnemyFromCam);
				float cosAngle = Math::Dot(dirToEnemy, camForward);

				// 船からの距離が弾の有効射程距離（lockOnMaxDistance_）以内の敵のみロックオンする！
				if (distToEnemy > 1.0f && distToEnemy <= lockOnMaxDistance_) {
					if (cosAngle > maxCos) {
						maxCos = cosAngle;
						lockOnTarget_ = obj.get();
					}
				}
			}
		}
	}

	// ロックオン中ならレティクルの色を鮮やかな赤にする（マテリアルカラーの変更）
	if (auto* renderer = gameObject_->GetComponent<MeshRendererComponent>()) {
		// レティクルは常に最前面に描画するためデプスを無効にする
		renderer->SetDepthEnable(false);
		// 最も手前（一番最後）に描画されるようにレイヤーを大きくする
		renderer->SetLayer(200);
		// アルファブレンドを有効にする
		renderer->SetBlendMode(MyEngine::Rendering::BlendModeType::Alpha);
		// ライトの影響を受けないようにする（暗闇で黒くならないようにする）
		renderer->SetEnableLighting(false);
		// 両面表示を有効にする（背面カリングで消えないようにする）
		renderer->SetDoubleSided(true);

		if (lockOnTarget_) {
			// ロックオン成功時：くっきり鮮やかな赤（アルファ0.95）
			renderer->SetColor({ 1.0f, 0.1f, 0.1f, 0.95f });
		} else {
			// 通常時：黒（半透明）
			renderer->SetColor({ 0.1f, 0.1f, 0.1f, 0.95f });
		}
	}
}

void ReticleComponent::ImGui() {
	ImGui::DragFloat("offsetZ", &offsetZ_, 0.01f);
	ImGui::DragFloat("LockOn Sensitivity", &lockOnAngleCos_, 0.0001f, 0.9000f, 1.0000f, "%.4f");
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