#pragma once
#include "Enemy.h"
#include "../Player/Player.h"
#include "object/3d/Model.h"
#include "MagosuyaEngine.h"

class EnemyManager
{
public:
	EnemyManager(MagosuyaEngine* engine);
public:
	void Initialize(Player* player);
	void Update(Matrix4x4* m);
	void Draw();
public:
	void Spawn(Matrix4x4* m);
	void SetPos(const Vector3& pos) { obj_->SetTransform({ {obj_->GetTransform().scale},{obj_->GetTransform().rotate},pos}); }
	void SetRot(const Vector3& rot) { obj_->SetTransform({ {obj_->GetTransform().scale},rot,{obj_->GetTransform().translate} }); }
	std::vector<std::unique_ptr<Enemy>>& GetEnemies() { return enemies_; }
private:
	void Delete();
private:
	// エンジン
	MagosuyaEngine* engine_ = nullptr;
	// プレイヤー
	Player* player_ = nullptr;
	// エネミー
	std::vector<std::unique_ptr<Enemy>>enemies_;
	// 生成インターバル
	float spawnInterval_;
	// 生成時間
	float spawnTimer_;
	// 生成最大数
	const uint32_t kMaxSpawnCount_ = 10;
	// 初期速度
	float initialSpeed_;// Enemyを射出する速度
	// 見た目
	std::unique_ptr<Model>obj_;
	// 出現する場所
	Vector3 spawnOffsetPos_;
};

