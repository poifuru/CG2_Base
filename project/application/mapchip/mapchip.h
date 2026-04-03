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
static inline const uint32_t kNumBlockVirtical = 100;
static inline const uint32_t kNumBlockHorizontal = 100;

struct EnemyPopData {
	MapChipType type;
	Vector3 position;
};

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

	// 配置すべき敵のリストを取得する
	const std::vector<EnemyPopData>& GetEnemyPopDatas() const { return enemyPopDatas_; }
	// リストをクリアする（生成し終わった後に呼ぶ用）
	void ClearEnemyPopDatas() { enemyPopDatas_.clear(); }

	//ゲッター
	uint32_t GetNumBlockVirtical() { return kNumBlockVirtical; }
	uint32_t GetNumBlockHorizontal() { return kNumBlockHorizontal; }

private:	//メンバ変数
	std::vector<MapChipType> mapData_;
	std::unique_ptr<MapChipRenderer> renderer_ = nullptr;

	std::vector<EnemyPopData> enemyPopDatas_;
};