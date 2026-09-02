#pragma once
#include "GameObject.h"

enum class BlockType {
	Pulock,		// 自然落下して隣同士でくっつく
	Fixed,		// 動かないブロック
	Goal,		// 触れたらゴール？

	Count
};

// ブロック1つのデータ
struct BlockData {
	int x, y;				// グリッド座標
	BlockType type;			// ブロックのタイプ
	GameObject* gameObject;	// ゲームオブジェクトへのポインタ
};