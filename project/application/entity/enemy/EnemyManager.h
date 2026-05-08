#pragma once
#include <vector>
#include <array>
#include <memory>
#include "struct.h"
#include "BaseEnemy.h"

class DxCommon;
class LightManager;

// 敵を出現させるためのデータ
struct SpawnCommand {
	float triggerZ;       // 出現させるプレイヤーのZ座標（または時間）
	int enemyType;        // 敵の種類（ザコ、中ボスなど）
	Vector3 startPos;     // 出現位置
	// PathData* path;    // 移動ルートのデータを持たせるとさらにGood!
};

// 敵の種類
enum EnemyType {
	kSmallFish = 0,
};


class EnemyManager {
public:
	static const int MAX_ENEMIES = 100;

	void Initialize(DxCommon* dxCommon, LightManager* light, CameraOrganizer* camera);

	// 毎フレーム、プレイヤーの進行度（Z座標など）を渡して更新する
	void Update(float playerProgressZ);
	void Draw();
	// プールから空いている敵を探して出現させる関数
	void SpawnEnemy(int type, const Vector3& pos);

	// アクセッサ
	const std::array<std::unique_ptr<BaseEnemy>, MAX_ENEMIES>& GetEnemies() const { return enemies_; }

private:
	// --- オブジェクトプール ---
	// BaseEnemyのポインタを最初から最大数持っておく
	std::array<std::unique_ptr<BaseEnemy>, MAX_ENEMIES> enemies_;

	// --- スポーン管理 ---
	std::vector<SpawnCommand> spawnTimeline_; // 敵の出現予定リスト（Z座標順に並べておく）
	int currentSpawnIndex_ = 0;               // 次にチェックする予定のインデックス

	// --- 生成に必要なポインタ ---
	DxCommon* dxCommon_ = nullptr;
	LightManager* light_ = nullptr;
	CameraOrganizer* camera_ = nullptr;
};