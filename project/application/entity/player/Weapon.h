#pragma once
#include <memory>
#include "struct.h"
#include "Model.h"
#include "Sprite.h"
#include "DxCommon.h"
#include "LightManager.h"
#include "CameraComponent.h"

class Weapon {
public:
	Weapon(DxCommon* dxCommon, LightManager* light);
	~Weapon();

	void Initialize();
	void Update(const Vector3& playerPos, float dirX, float dirY, bool isGrounded, CameraData* camera);
	void Draw();
	void ImGui();

	// 攻撃開始命令
	void Attack();

	// 敵との当たり判定チェック
	bool CheckCollision(const AABB& enemyAABB);

	//アクセッサー
	bool IsAttacking() const { return isAttacking_; }
	const AABB& GetAttackAABB() const { return attackAABB_; }

private:
	std::unique_ptr<Model> model_ = nullptr;

	Transform transform_;
	AABB localAABB_;      // 武器自体の当たり判定（大きさ）
	AABB attackAABB_;     // ワールド空間での当たり判定（計算結果）

	bool isAttacking_ = false;
	int attackTimer_ = 0;
	int attackCooldown_ = 0;
};