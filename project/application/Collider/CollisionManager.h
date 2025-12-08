#pragma once
#include "Collider.h"
#include <list>
#include <vector>

class CollisionManager
{
public:
	void Begin();
	void SetColliders(Collider* collider);
	void CheckAllCollisions();
private:
	void CheckCollisionPair(Collider* colliderA, Collider* colliderB);
private:
	std::vector<Collider*> colliders_;
};


