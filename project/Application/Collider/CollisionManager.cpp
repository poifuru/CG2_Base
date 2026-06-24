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

	// 総当たり判定
	for(size_t i = 0; i < objects_.size() - 1; ++i) {
		for(size_t j = 0; j < objects_.size(); ++j) {
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
	// 両方とも球の場合
	if (a->GetCollisionType() == CollisionType::Sphere && 
		b->GetCollisionType() == CollisionType::Sphere)
	{
		if (Math::IsCollision(a->GetSphere(), b->GetSphere()))
		{
			a->OnCollision(b);
			b->OnCollision(a);
		}
	}
	// 両方ともAABBの場合
	else if (a->GetCollisionType() == CollisionType::AABB && 
			 b->GetCollisionType() == CollisionType::AABB)
	{
		if (Math::IsCollision(a->GetAABB(), b->GetAABB()))
		{
			a->OnCollision(b);
			b->OnCollision(a);
		}
	}
	// AABB と 球 の場合（aがAABB、bが球）
	else if (a->GetCollisionType() == CollisionType::AABB && 
			 b->GetCollisionType() == CollisionType::Sphere)
	{
		if (Math::IsCollision(a->GetAABB(), b->GetSphere()))
		{
			a->OnCollision(b);
			b->OnCollision(a);
		}
	}
	// AABB と 球 の場合（aが球、bがAABB）
	else if (a->GetCollisionType() == CollisionType::Sphere && 
			 b->GetCollisionType() == CollisionType::AABB)
	{
		// 引数の順番を AABB, Sphere に合わせて呼び出す
		if (Math::IsCollision(b->GetAABB(), a->GetSphere()))
		{
			a->OnCollision(b);
			b->OnCollision(a);
		}
	}
	return false;
}
