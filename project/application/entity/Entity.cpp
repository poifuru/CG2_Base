#include "Entity.h"
#include <algorithm> // std::min, std::max 用

void Entity::CheckMapCollision(MapChip* mapChip) {
	const float kMargin = 0.001f; // 完全に0にせず、ごくわずかな隙間を作る

	// --- 1. X軸方向の移動と壁判定 ---
	transform_.translate.x += velocity_.x;

	UpdateAABB();

	float worldLeft = aabb_.min.x;
	float worldRight = aabb_.max.x;
	float worldBottom = aabb_.min.y;
	float worldTop = aabb_.max.y;

	IndexSet minIdx = mapChip->GetMapChipIndexSetByPosition({ worldLeft, worldBottom, 0.0f });
	IndexSet maxIdx = mapChip->GetMapChipIndexSetByPosition({ worldRight, worldTop, 0.0f });

	uint32_t loopStartY = (std::min)(minIdx.yIndex, maxIdx.yIndex);
	uint32_t loopEndY = (std::max)(minIdx.yIndex, maxIdx.yIndex);
	uint32_t loopStartX = (std::min)(minIdx.xIndex, maxIdx.xIndex);
	uint32_t loopEndX = (std::max)(minIdx.xIndex, maxIdx.xIndex);

	isTouchingWallLeft_ = false;
	isTouchingWallRight_ = false;

	for(uint32_t x = loopStartX; x <= loopEndX; ++x) {
		for(uint32_t y = loopStartY; y <= loopEndY; ++y) {
			MapChipType type = mapChip->GetMapChipTypeByIndex(x, y);
			if(type == MapChipType::kWall || type == MapChipType::kFloor || type == MapChipType::kCeiling) {
				if(velocity_.x > 0.0f) {
					Rect tileRect = mapChip->GetRectByIndex(x, y);
					transform_.translate.x = tileRect.left - aabbSize_.x - kMargin;
					isTouchingWallRight_ = true; // ★右壁フラグ
				}
				else if(velocity_.x < 0.0f) {
					Rect tileRect = mapChip->GetRectByIndex(x, y);
					transform_.translate.x = tileRect.right + aabbSize_.x + kMargin;
					isTouchingWallLeft_ = true; // ★左壁フラグ	
				}
				velocity_.x = 0.0f;

				UpdateAABB();

				goto breakX;
			}
		}
	}
breakX:

	// --- 2. Y軸方向の移動と床・天井判定 ---
	isGrounded_ = false; // 判定前にリセット
	transform_.translate.y += velocity_.y;
	UpdateAABB();

	// ★ここで「X軸で押し戻された後」の最新のワールド座標を使い直す
	worldLeft = aabb_.min.x;
	worldRight = aabb_.max.x;
	worldBottom = aabb_.min.y;
	worldTop = aabb_.max.y;

	// インデックスも再計算！
	minIdx = mapChip->GetMapChipIndexSetByPosition({ worldLeft, worldBottom, 0.0f });
	maxIdx = mapChip->GetMapChipIndexSetByPosition({ worldRight, worldTop, 0.0f });

	loopStartY = (std::min)(minIdx.yIndex, maxIdx.yIndex);
	loopEndY = (std::max)(minIdx.yIndex, maxIdx.yIndex);
	loopStartX = (std::min)(minIdx.xIndex, maxIdx.xIndex);
	loopEndX = (std::max)(minIdx.xIndex, maxIdx.xIndex);

	for(uint32_t x = loopStartX; x <= loopEndX; ++x) {
		for(uint32_t y = loopStartY; y <= loopEndY; ++y) {
			MapChipType type = mapChip->GetMapChipTypeByIndex(x, y);
			if(type != MapChipType::kBlank) {
				if(velocity_.y > 0.0f) { // 天井ヒット
					Rect tileRect = mapChip->GetRectByIndex(x, y);
					transform_.translate.y = tileRect.bottom - aabbSize_.y - kMargin;
				}
				else if(velocity_.y < 0.0f) { // 地面ヒット
					Rect tileRect = mapChip->GetRectByIndex(x, y);
					transform_.translate.y = tileRect.top + aabbSize_.y + kMargin;
					isGrounded_ = true;

					// ★追加：踏んだタイルがダメージ床（kDamage）ならフラグを立てる
					if(type == MapChipType::kDamage) {
						isOnDamageFloor_ = true;
					}

					if(type == MapChipType::kGoal) {
						isGoalReached_ = true;
					}
				}
				velocity_.y = 0.0f;
				goto breakY;
			}
		}
	}
breakY:
	UpdateAABB();
	return;
}

void Entity::UpdateAABB() {
	// 現在の座標(transform_.translate)を中心に、aabbSize_ 分だけ広げる
	aabb_.min = {
		transform_.translate.x - aabbSize_.x,
		transform_.translate.y - aabbSize_.y,
		transform_.translate.z - aabbSize_.z
	};
	aabb_.max = {
		transform_.translate.x + aabbSize_.x,
		transform_.translate.y + aabbSize_.y,
		transform_.translate.z + aabbSize_.z
	};
}