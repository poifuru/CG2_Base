#pragma once
#include <vector>
#include "Collision.h"

class CollisionManager {
public:
	// オブジェクトの登録
	void RegisterObject(CollisionObject* obj);

	// オブジェクトの登録解除
	void UnregisterObject(CollisionObject* obj);

	// ゲームループで呼び出す更新処理
	void UpdateAllCollisions();

private:
	// 形状を見て判定を分岐する関数
	bool CheckActualCollision(CollisionObject* a, CollisionObject* b);

private:
	std::vector<CollisionObject*> objects_;
};