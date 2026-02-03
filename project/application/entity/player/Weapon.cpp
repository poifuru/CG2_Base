#include "Weapon.h"
#include <numbers>
#include "imgui.h"

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
	Vector3 rotate = {};

	model_->SetRotate(rotate);
}

void Weapon::Update(const Vector3& playerPos, float dirX, float dirY, bool isGrounded, CameraData* camera) {
	if(attackCooldown_ > 0) attackCooldown_--;

	// 使う方向を決定（攻撃中ならロックされた方向、そうでなければ今の入力方向）
	float useDirX = isAttacking_ ? lockedDirX_ : dirX;
	float useDirY = isAttacking_ ? lockedDirY_ : dirY;

	// プレイヤーからどれくらい離すか（重ならない距離）
	const float kOffsetDist = 2.5f;
	Vector3 offset = { 0, 0, 0 };
	Vector3 rotate = { 
		0.0f,
		std::numbers::pi_v<float> / -2.0f,
		0.0f
	}; // ここで角度を決めるでやんす

	float PI = std::numbers::pi_v<float>;

	// --- 向きと位置の計算 ---
	if(useDirY > 0.1f) { // 上攻撃
		offset.y = kOffsetDist;
		rotate.x = PI;
		rotate.z = PI; // 90度（上を向く）
	}
	else if(useDirY < -0.1f && !isGrounded) { // 下攻撃
		offset.y = -kOffsetDist;
		rotate.z = -PI; // -90度（下を向く）
	}
	else { // 左右攻撃
		offset.x = useDirX * kOffsetDist;
		// 右向きなら 0度、左向きなら 180度
		rotate.z = (useDirX < 0) ? PI / 2.0f : PI / -2.0f;
	}

	// --- 当たり判定 AABB の更新 ---
	if(isAttacking_) {
		// オフセットを足してプレイヤーの周りに配置
		attackAABB_.min = { playerPos.x + offset.x - 1.0f, playerPos.y + offset.y - 1.0f, -1.0f };
		attackAABB_.max = { playerPos.x + offset.x + 1.0f, playerPos.y + offset.y + 1.0f,  1.0f };

		attackTimer_--;
		if(attackTimer_ <= 0) isAttacking_ = false;
	}

	model_->SetPosition({ playerPos.x + offset.x, playerPos.y + offset.y, playerPos.z });
	model_->SetRotate(rotate);
	model_->Update(camera);
}
void Weapon::Draw() {
	if(isAttacking_) model_->Draw();
}

void Weapon::ImGui() {
#ifdef USEIMGUI
	model_->ImGui("hammer");
	ImGui::Text("Attack AABB Min: %.2f, %.2f, %.2f", attackAABB_.min.x, attackAABB_.min.y, attackAABB_.min.z);
	ImGui::Text("Attack AABB Max: %.2f, %.2f, %.2f", attackAABB_.max.x, attackAABB_.max.y, attackAABB_.max.z);
	ImGui::Separator();
#endif
}

void Weapon::Attack(float currentDirX, float currentDirY) {
	if(attackCooldown_ <= 0) {
		isAttacking_ = true;
		attackTimer_ = 8;     // 判定が出る時間は短めが「鋭い」でやんす！
		attackCooldown_ = 15;
	}

	// ★攻撃を出した瞬間の向きをロックするでやんす！
	lockedDirX_ = currentDirX;
	lockedDirY_ = currentDirY;
}

bool Weapon::CheckCollision(const AABB& enemyAABB) {
	// 攻撃中でなければ判定しない
	if(!isAttacking_) return false;

	// AABB同士の交差判定（X, Y, Zすべてが重なっているか）
	if(attackAABB_.min.x <= enemyAABB.max.x && attackAABB_.max.x >= enemyAABB.min.x &&
	   attackAABB_.min.y <= enemyAABB.max.y && attackAABB_.max.y >= enemyAABB.min.y &&
	   attackAABB_.min.z <= enemyAABB.max.z && attackAABB_.max.z >= enemyAABB.min.z) {
		return true;
	}

	return false;
}
