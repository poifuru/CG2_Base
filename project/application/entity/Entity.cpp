#include "Entity.h"
#include <algorithm> // std::min, std::max 用

void Entity::CheckMapCollision(MapChip* mapChip) {
	const float kMargin = 0.001f; // 完全に0にせず、ごくわずかな隙間を作る

	// --- 1. X軸方向の移動と壁判定 ---
	transform_.translate.x += velocity_.x;

	float worldLeft = transform_.translate.x + aabb_.min.x;
	float worldRight = transform_.translate.x + aabb_.max.x;
	float worldBottom = transform_.translate.y + aabb_.min.y;
	float worldTop = transform_.translate.y + aabb_.max.y;

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
					transform_.translate.x = tileRect.left - aabb_.max.x - kMargin;
					isTouchingWallRight_ = true; // ★右壁フラグ
				}
				else if(velocity_.x < 0.0f) {
					Rect tileRect = mapChip->GetRectByIndex(x, y);
					transform_.translate.x = tileRect.right - aabb_.min.x + kMargin;
					isTouchingWallLeft_ = true; // ★左壁フラグ	
				}
				velocity_.x = 0.0f;
				goto breakX;
			}
		}
	}
breakX:

	// --- 2. Y軸方向の移動と床・天井判定 ---
	isGrounded_ = false; // 判定前にリセット
	transform_.translate.y += velocity_.y;

	// ★ここで「X軸で押し戻された後」の最新のワールド座標を使い直すでやんす！
	worldLeft = transform_.translate.x + aabb_.min.x;
	worldRight = transform_.translate.x + aabb_.max.x;
	worldBottom = transform_.translate.y + aabb_.min.y;
	worldTop = transform_.translate.y + aabb_.max.y;

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
					transform_.translate.y = tileRect.bottom - aabb_.max.y - kMargin;
				}
				else if(velocity_.y < 0.0f) { // 地面ヒット
					Rect tileRect = mapChip->GetRectByIndex(x, y);
					transform_.translate.y = tileRect.top - aabb_.min.y + kMargin;
					isGrounded_ = true;
				}
				velocity_.y = 0.0f;
				goto breakY;
			}
		}
	}
breakY:
	return;
}