#pragma once
#include "BlockData.h"

class BlockEditor {
public:
	void Add(int x, int y, BlockType type);

	void Remove(int x, int y);
};