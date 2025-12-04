#include "CollisionManager.h"
#include "MathFunction.h"

void CollisionManager::Begin() {
	colliders_.clear();
}

void CollisionManager::SetColliders(Collider* collider) {
	colliders_.push_back(collider);
}

void CollisionManager::CheckAllCollisions() {

	for (size_t i = 0;i < colliders_.size();++i) {
		Collider* colliderA = colliders_[i];
		for (size_t j = i + 1;j < colliders_.size();++j) {
			Collider* colliderB = colliders_[j];
			CheckCollisionPair(colliderA, colliderB);
		}
	}

	/*std::list<Collider*>::iterator itrA = colliders_.begin();
	for (;itrA != colliders_.end();++itrA) {
		Collider* colliderA = *itrA;

		std::list<Collider*>::iterator itrB = itrA;
		itrB++;
		for (;itrB != colliders_.end();++itrB) {
			Collider* colliderB = *itrB;

			CheckCollisionPair(colliderA, colliderB);
		}
	}*/
}

void CollisionManager::CheckCollisionPair(Collider* colliderA, Collider* colliderB) {
	if ((colliderA->GetMyType() & colliderB->GetYourType()) == 0 ||
		(colliderB->GetMyType() & colliderA->GetYourType()) == 0) {
		return;
	}

	Vector3 posA = colliderA->GetWorldPosition();
	Vector3 posB = colliderB->GetWorldPosition();
	float length = Length(posA - posB);
	if (colliderA->GetRadius() + colliderB->GetRadius() >= length) {
		colliderA->OnCollision(colliderB);
		colliderB->OnCollision(colliderA);
	}
}