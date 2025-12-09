#pragma once
#include "Collider.h"

// Bossクラスが未定義の場合のフォワードデクレア
class Boss;

// Boss本体の当たり判定を担うクラス
class BossBodyCollider final :
	public Collider // Colliderクラスを継承
{
public:
	// コンストラクタ: 親となるBossのポインタを受け取る
	BossBodyCollider(Boss* boss);

	// 衝突が発生した際の処理 (Colliderクラスの仮想関数をオーバーライド)
	void OnCollision(Collider* other) override;

	// Collider 継承の必須メソッド: 自身のワールド座標を返す
	const Vector3 GetWorldPosition() override;

private:
	Boss* boss_; // 親となるBossへのポインタ
};