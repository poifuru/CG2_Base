#include "PCH.h"
#include "CollisionManager.h"
#include "MathFunction.h"

void CollisionManager::RegisterObject(CollisionObject* obj) {
	objects_.push_back(obj);
}

void CollisionManager::UnregisterObject(CollisionObject* obj) {
	for(auto it = objects_.begin(); it != objects_.end(); ++it) {
		if(*it == obj) {
			objects_.erase(it);
			break;
		}
	}
}

void CollisionManager::UpdateAllCollisions() {
	if(objects_.size() < 2) return;

	// 重複のない総当たり判定（i < j）にすることで自分自身や重複判定を防ぐ
	for(size_t i = 0; i < objects_.size() - 1; ++i) {
		for(size_t j = i + 1; j < objects_.size(); ++j) {
			CollisionObject* a = objects_[i];
			CollisionObject* b = objects_[j];
			if(CheckActualCollision(a, b)) {
				// 当たっていたら互いに衝突を通知する
				a->OnCollision(b);
				b->OnCollision(a);
			}
		}
	}
}

bool CollisionManager::CheckActualCollision(CollisionObject* a, CollisionObject* b) {
	// 関数内での OnCollision 呼び出しをやめて、純粋に判定結果（bool）だけを返す！
	if (a->GetCollisionType() == CollisionType::Sphere && 
		b->GetCollisionType() == CollisionType::Sphere)
	{
		return Math::IsCollision(a->GetSphere(), b->GetSphere());
	}
	else if (a->GetCollisionType() == CollisionType::AABB && 
			 b->GetCollisionType() == CollisionType::AABB)
	{
		return Math::IsCollision(a->GetAABB(), b->GetAABB());
	}
	else if (a->GetCollisionType() == CollisionType::AABB && 
			 b->GetCollisionType() == CollisionType::Sphere)
	{
		return Math::IsCollision(a->GetAABB(), b->GetSphere());
	}
	else if (a->GetCollisionType() == CollisionType::Sphere && 
			 b->GetCollisionType() == CollisionType::AABB)
	{
		return Math::IsCollision(b->GetAABB(), a->GetSphere());
	}
	return false;
}
