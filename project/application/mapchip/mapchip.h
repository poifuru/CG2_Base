#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include "struct.h"

//1ブロックのサイズ
static inline const float kBlockWidth = 2.0f;
static inline const float kBlockHeight = 2.0f;
//ブロックの個数
static inline const uint32_t kNumBlockVirtical = 40;
static inline const uint32_t kNumBlockHorizontal = 80;

//ブロックの種類
enum class MapChipType {
	kBlank,		//空白
	kFloor,		//床
	kWall,		//壁
	kCeiling	//天井
};

struct IndexSet {
	uint32_t xIndex;
	uint32_t yIndex;
};

struct Rect {
	float left;		//左端
	float right;	//右端
	float bottom;	//下端
	float top;		//上端
};

class MapChip {
public:		//メンバ関数
	void ResetMapChipData();

	void LoadMapChipCSV(const std::string& filePath);

	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);

	Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

	IndexSet GetMapChipIndexSetByPosition(const Vector3& position);

	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);

	//ゲッター
	uint32_t GetNumBlockVirtical() { return kNumBlockVirtical; }
	uint32_t GetNumBlockHorizontal() { return kNumBlockHorizontal; }

private:	//メンバ変数
	std::vector<MapChipType> mapData_;
};