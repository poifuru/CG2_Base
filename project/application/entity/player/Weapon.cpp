#include "Weapon.h"
#include <numbers>

Weapon::Weapon(DxCommon* dxCommon, LightManager* light) {
	model_ = std::make_unique<Model>(dxCommon, light);
}

Weapon::~Weapon() {

}

void Weapon::Initialize() {
	model_->Initialize();
	model_->SetModelData("hammer.obj");
	model_->SetTexture("hammer");

	//角度調整
	Vector3 rotate = {
		0.0f,
		std::numbers::pi_v<float> / 2.0f,
		0.0f
	};

	model_->SetRotate(rotate);
}

void Weapon::Update(const Vector3& playerPos, float dirX, float dirY, bool isGrounded, CameraData* camera) {
	if(attackCooldown_ > 0) attackCooldown_--;

	// 武器を配置するオフセット距離（重ならないように調整）
	const float kOffsetDist = 1.5f;
	Vector3 offset = { 0, 0, 0 };
	Vector3 rotate = { 0, 0, 0 };

	float PI = std::numbers::pi_v<float>;

	// 優先順位：上入力 > 下入力 > 左右入力
	if(dirY > 0.1f) { // 上攻撃
		offset.y = kOffsetDist;
		rotate.z = PI / 2.0f; // 90度（上向き）
	}
	else if(dirY < -0.1f && !isGrounded) { // 下攻撃（空中の時だけとか）
		offset.y = -kOffsetDist;
		rotate.z = -PI / 2.0f; // -90度（下向き）
	}
	else { // 左右攻撃
		offset.x = dirX * kOffsetDist;
		// 左向き(-1.0)なら180度回して反転させるでやんす
		rotate.y = (dirX < 0) ? PI : 0.0f;
	}

	// 攻撃中だけ当たり判定のワールド座標を計算
	if(isAttacking_) {
		attackAABB_.min = { playerPos.x + offset.x + localAABB_.min.x, playerPos.y + offset.y + localAABB_.min.y, -1.0f };
		attackAABB_.max = { playerPos.x + offset.x + localAABB_.max.x, playerPos.y + offset.y + localAABB_.max.y,  1.0f };

		attackTimer_--;
		if(attackTimer_ <= 0) isAttacking_ = false;
	}

	model_->SetPosition(playerPos);
	model_->SetRotate(rotate);
	model_->Update(camera);
}
void Weapon::Draw() {
	if(isAttacking_) model_->Draw();
}

void Weapon::ImGui() {
	model_->ImGui("hammer");
}

void Weapon::Attack() {
	if(attackCooldown_ <= 0) {
		isAttacking_ = true;
		attackTimer_ = 8;     // 判定が出る時間は短めが「鋭い」でやんす！
		attackCooldown_ = 15;
	}
}

bool Weapon::CheckCollision(const AABB& enemyAABB) {
	return false;
}
