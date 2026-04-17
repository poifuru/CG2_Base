#include "Fly.h"
#include <cmath>
#include <numbers>
#include "CameraOrganizer.h"

Fly::Fly(DxCommon* dxCommon, LightManager* light, MapChip* mapchip)
	: BaseEnemy(dxCommon, light, mapchip) {
}

void Fly::Initialize() {
	// 初期位置を基準の高さとして保存しておく
	baseHeight_ = transform_.translate.y;
	hp_ = 1; // 体力の設定
	SetAABBSize({ 0.5f, 0.5f, 0.5f }); // 当たり判定のサイズ

	model_->SetModelData("korokoro.obj"); // 敵のモデル
	model_->SetTexture("korokoro");
	model_->Initialize();
	model_->SetRoughness(0.6f);
	model_->SetMetallic(0.0f);

	model_->SetRotate({ 0.0f, std::numbers::pi_v<float> / -2.0f, 0.0f });
}

void Fly::Update() {
	// 死亡演出などの共通処理は親クラスに任せる
	BaseEnemy::Update();

	// 生きている時だけふよふよ動く
	if(!isDead_) {
		// --- ふよふよ移動の計算 ---
		floatingTimer_ += 0.05f; // 数値を大きくすると揺れが速くなる

		float amplitude = 0.5f; // 揺れ幅（どれくらい上下するか）

		// 基準の高さにサイン波を足すことで上下に揺らす
		transform_.translate.y = baseHeight_ + std::sin(floatingTimer_) * amplitude;

		// AABB（当たり判定）の更新
		UpdateAABB();

		// モデルの更新
		model_->SetPosition(transform_.translate);
		model_->Update(&CameraOrganizer::GetInstance()->GetCameraData());
	}
}