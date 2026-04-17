#include "Korokoro.h"
#include "CameraOrganizer.h"
#include "imgui.h"

Korokoro::Korokoro(DxCommon* dxCommon, LightManager* light, MapChip* mapchip)
	: BaseEnemy(dxCommon, light, mapchip) {
	hp_ = 3;
	walkSpeed_ = 0.1f;
}

Korokoro::~Korokoro() {

}

void Korokoro::Initialize() {
	model_->SetModelData("korokoro.obj"); // 敵のモデル
	model_->SetTexture("korokoro");
	model_->Initialize();
	model_->SetRoughness(0.6f);
	model_->SetMetallic(0.0f);

	// 判定サイズの設定
	SetAABBSize({ 1.0f, 1.0f, 1.0f });
	velocity_.x = walkSpeed_;
}

void Korokoro::Update() {
	if(hp_ <= 0 && alpha_ <= 0.0f) return;

	BaseEnemy::Update();

	// 重力
	velocity_.y += -0.01f;

	// 壁に当たったら反転
	if(isTouchingWallLeft_) {
		velocity_.x = walkSpeed_;
	}
	else if(isTouchingWallRight_) {
		velocity_.x = -walkSpeed_;
	}

	// 物理挙動と当たり判定の実行
	CheckMapCollision(mapchip_);

	// モデルの更新
	model_->SetPosition(transform_.translate);
	model_->Update(&CameraOrganizer::GetInstance()->GetCameraData());
}

void Korokoro::ImGui() {
	model_->ImGui("korokoro");
	ImGui::Text("HP : %d", hp_);
	ImGui::Text("aabb Min: %.2f, %.2f, %.2f", aabb_.min.x, aabb_.min.y, aabb_.min.z);
	ImGui::Text("aabb Max: %.2f, %.2f, %.2f", aabb_.max.x, aabb_.max.y, aabb_.max.z);
}