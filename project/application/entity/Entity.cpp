#include "Entity.h"

void Entity::CheckMapCollision(MapChip* mapChip) {
	// わずかなめり込みを許容するためのマージン
	const float kMargin = 0.01f;

	// --- 1. X軸方向の移動と壁判定 ---
	transform_.translate.x += velocity_.x;

	// AABBの現在の範囲（ワールド座標）を算出
	float worldLeft = transform_.translate.x + aabb_.min.x;
	float worldRight = transform_.translate.x + aabb_.max.x;
	float worldBottom = transform_.translate.y + aabb_.min.y;
	float worldTop = transform_.translate.y + aabb_.max.y;

	// AABBがカバーするタイルのインデックス範囲を算出
	IndexSet minIdx = mapChip->GetMapChipIndexSetByPosition({ worldLeft, worldBottom, 0.0f });
	IndexSet maxIdx = mapChip->GetMapChipIndexSetByPosition({ worldRight, worldTop, 0.0f });

	// Yのインデックスは上下逆転しているので、ループの範囲に注意
	uint32_t startY = maxIdx.yIndex; // 上端（インデックス小）
	uint32_t endY = minIdx.yIndex; // 下端（インデックス大）

	for (uint32_t x = minIdx.xIndex; x <= maxIdx.xIndex; ++x) {
		for (uint32_t y = startY; y <= endY; ++y) {
			if (mapChip->GetMapChipTypeByIndex(x, y) == MapChipType::kWall) {
				// 壁に衝突した場合の押し戻し処理
				if (velocity_.x > 0.0f) { // 右移動中
					Rect tileRect = mapChip->GetRectByIndex(x, y);
					transform_.translate.x = tileRect.left - aabb_.max.x - kMargin;
				}
				else if (velocity_.x < 0.0f) { // 左移動中
					Rect tileRect = mapChip->GetRectByIndex(x, y);
					transform_.translate.x = tileRect.right - aabb_.min.x + kMargin;
				}
				velocity_.x = 0.0f;
				goto breakX; // X方向の補正が終わればこの軸のループは抜ける
			}
		}
	}
breakX:

	// --- 2. Y軸方向の移動と床・天井判定 ---
	transform_.translate.y += velocity_.y;

	// Y移動後の最新のAABB範囲を再計算
	worldLeft = transform_.translate.x + aabb_.min.x;
	worldRight = transform_.translate.x + aabb_.max.x;
	worldBottom = transform_.translate.y + aabb_.min.y;
	worldTop = transform_.translate.y + aabb_.max.y;

	minIdx = mapChip->GetMapChipIndexSetByPosition({ worldLeft, worldBottom, 0.0f });
	maxIdx = mapChip->GetMapChipIndexSetByPosition({ worldRight, worldTop, 0.0f });

	startY = maxIdx.yIndex;
	endY = minIdx.yIndex;

	for (uint32_t x = minIdx.xIndex; x <= maxIdx.xIndex; ++x) {
		for (uint32_t y = startY; y <= endY; ++y) {
			MapChipType type = mapChip->GetMapChipTypeByIndex(x, y);
			if (type != MapChipType::kBlank) {
				// 空白以外（床・壁・天井）に衝突
				if (velocity_.y > 0.0f) { // 上昇（天井にヒット）
					Rect tileRect = mapChip->GetRectByIndex(x, y);
					transform_.translate.y = tileRect.bottom - aabb_.max.y - kMargin;
				}
				else if (velocity_.y < 0.0f) { // 下降（地面にヒット）
					Rect tileRect = mapChip->GetRectByIndex(x, y);
					transform_.translate.y = tileRect.top - aabb_.min.y + kMargin;
				}
				velocity_.y = 0.0f;
				goto breakY;
			}
		}
	}
breakY:
	return;
}