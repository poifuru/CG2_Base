#include "Mapchip.h"
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <assert.h>

namespace {
	std::map<std::string, MapChipType> mapChipTable{
		{"0", MapChipType::kBlank},		//空白
		{"1", MapChipType::kFloor},		//床
		{"2", MapChipType::kWall},		//壁
		{"3", MapChipType::kCeiling},	//天井
		{"4", MapChipType::kDamage},	//天井
		{"5", MapChipType::kGoal},
		{"8", MapChipType::kKorokoro },
		{"9", MapChipType::kFly },
	};
}

MapChip::MapChip() {
	renderer_ = std::make_unique<MapChipRenderer>();
}

MapChip::~MapChip() {

}

void MapChip::Initialize(DxCommon* dxCommon, LightManager* lightManager) {
	renderer_->Initialize(dxCommon, lightManager);
}

void MapChip::Update(const Matrix4x4& vp, Vector3 cameraWorld) {
	renderer_->Update(*this, vp, cameraWorld);
}

void MapChip::Draw() {
	renderer_->Draw();
}

void MapChip::ImGui(const std::string& name) {
	renderer_->ImGui(name);
}

void MapChip::ResetMapChipData() {
	// 全体のサイズ分確保（縦 × 横）
	mapData_.clear();
	mapData_.assign(kNumBlockVirtical * kNumBlockHorizontal, MapChipType::kBlank);
}

void MapChip::LoadMapChipCSV(const std::string& filePath) {
	//マップチップデータをリセット
	ResetMapChipData();
	enemyPopDatas_.clear();

	//ファイルを開く
	std::ifstream file(filePath);
	assert(file.is_open());

	//CSVから1行ずつ読み込む
	for(uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		std::string line;
		//ファイルの終わりならfor文を抜ける
		if(!std::getline(file, line)) break;
		std::istringstream line_stream(line);

		for(uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			std::string word;
			//行の終わりならfor文を抜ける
			if(!std::getline(line_stream, word, ',')) break;

			const char* trimChars = " \t\v\r\n";
			size_t left = word.find_first_not_of(trimChars);
			if(left != std::string::npos) {
				size_t right = word.find_last_not_of(trimChars);
				word = word.substr(left, right - left + 1);
			}
			else {
				word = "";
			}

			//wordに対応するマップチップ種類があるか確認
			if(mapChipTable.contains(word)) {
				MapChipType type = mapChipTable[word];

				// ★ここから追加
				if(type == MapChipType::kKorokoro || type == MapChipType::kFly) {
					// 敵の場合は座標を計算してポップリストに入れる
					Vector3 pos = GetMapChipPositionByIndex(j, i);
					enemyPopDatas_.push_back({ type, pos });

					// マップ自体には「空白」として登録しておく（描画させないため）
					uint32_t index = i * kNumBlockHorizontal + j;
					mapData_[index] = MapChipType::kBlank;
				}
				else {
					// 通常のブロックは今まで通り
					uint32_t index = i * kNumBlockHorizontal + j;
					mapData_[index] = type;
				}
				// ★ここまで追加
			}
		}
	}
	file.close();
}

MapChipType MapChip::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) {
	//範囲外チェック
	if(xIndex >= kNumBlockHorizontal || yIndex >= kNumBlockVirtical) {
		//範囲外なら強制的にkBlankを返す
		return MapChipType::kBlank;
	}

	//一次元配列に計算式でアクセス
	return mapData_[yIndex * kNumBlockHorizontal + xIndex];
}

Vector3 MapChip::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) {
	return Vector3(kBlockWidth * xIndex, kBlockHeight * (kNumBlockVirtical - 1 - yIndex), 0.0f);
}

IndexSet MapChip::GetMapChipIndexSetByPosition(const Vector3& position) {
	IndexSet indexSet = {};
	indexSet.xIndex = uint32_t((position.x + kBlockWidth / 2.0f) / kBlockWidth);
	//y座標反転前の番号
	uint32_t yIndexBefore = uint32_t((position.y + kBlockHeight / 2.0f) / kBlockHeight);
	indexSet.yIndex = uint32_t(kNumBlockVirtical - 1 - yIndexBefore);
	return indexSet;
}

Rect MapChip::GetRectByIndex(uint32_t xIndex, uint32_t yIndex) {
	Vector3 center = GetMapChipPositionByIndex(xIndex, yIndex);

	Rect rect;
	rect.left = center.x - kBlockWidth / 2.0f;
	rect.right = center.x + kBlockWidth / 2.0f;
	rect.bottom = center.y - kBlockHeight / 2.0f;
	rect.top = center.y + kBlockHeight / 2.0f;

	return rect;
};