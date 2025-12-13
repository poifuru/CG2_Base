#pragma once
#include "Collider.h"

class Enemy;

class EnemyBodyCollider
	:public Collider // Colliderクラスを継承
{
public:
	// コンストラクタ: 親となるPlayerのポインタを受け取る
	EnemyBodyCollider(Enemy* enemy);

	//  衝突が発生した際の処理
	void OnCollision(Collider* other) override;

	// Collider 継承の必須メソッド: 自身のワールド座標を返す
	const Vector3 GetWorldPosition() override;

private:
	Enemy* enemy_; // 親となるPlayerへのポインタ
};

