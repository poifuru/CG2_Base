#pragma once
#include "Enemy.h"
#include "Player.h"
#include "Boss.h"
#include "Model.h"
#include "DxCommon.h"

class EnemyManager
{
public:
	EnemyManager(DxCommon* dxCommon);
public:
	void Initialize(Player* player, Boss* boss);
	void ResetData();
	void Update(Matrix4x4* m);
	void Draw();
public:
	void Spawn(Matrix4x4* m);
	void SetPos(const Vector3& pos) { obj_->SetTransform({ {obj_->GetTransform().scale},{obj_->GetTransform().rotate},pos}); }
	void SetRot(const Vector3& rot) { obj_->SetTransform({ {obj_->GetTransform().scale},rot,{obj_->GetTransform().translate} }); }
	std::vector<std::unique_ptr<Enemy>>& GetEnemies() { return enemies_; }
private:
	void Delete();
	void ChangeState();
private:
	// エンジン
	DxCommon* dxCommon_ = nullptr;
	// プレイヤー
	Player* player_ = nullptr;
	// ボス
	Boss* boss_ = nullptr;
	// エネミー
	std::vector<std::unique_ptr<Enemy>>enemies_;
	// 生成インターバル
	float spawnInterval_;
	// 生成時間
	float spawnTimer_;
	// 生成最大数
	const uint32_t kMaxSpawnCount_ = 15;
	// 初期速度
	float initialSpeed_;// Enemyを射出する速度
	// 見た目
	std::unique_ptr<Model>obj_;
	// 出現する場所
	Vector3 spawnOffsetPos_;
	// [ エリアのサイズ ]
	Vector3 ariaLeftTop_ = {-45.0f,0.0f,45.0f};
	Vector2 ariaSize_ = {90.0f,-90.0f};
};

