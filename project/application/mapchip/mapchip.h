#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include "MapchipStruct.h"
#include "mapChipRenderer.h"

//1ブロックのサイズ
static inline const float kBlockWidth = 2.0f;
static inline const float kBlockHeight = 2.0f;
//ブロックの個数
static inline const uint32_t kNumBlockVirtical = 20;
static inline const uint32_t kNumBlockHorizontal = 100;

class MapChip {
public:		//メンバ関数
	MapChip();
	~MapChip();

	void Initialize(DxCommon* dxCommon, LightManager* lightManager);
	void Update(const Matrix4x4& vp, Vector3 cameraWorld);
	void Draw();
	void ImGui(const std::string& name);

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
	std::unique_ptr<MapChipRenderer> renderer_ = nullptr;
};