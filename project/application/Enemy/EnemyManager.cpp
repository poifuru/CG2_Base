#include "EnemyManager.h"
#include "MathFunction.h"
#include "ModelManager.h"
#include <algorithm>
#include "imgui.h"

EnemyManager::EnemyManager(DxCommon* dxCommon) {
	dxCommon_ = dxCommon;
	obj_ = std::make_unique<Model>(dxCommon);
	ModelManager::GetInstance ()->LoadModelData ("Resources/zako", "zako");
	ModelManager::GetInstance ()->LoadModelData ("Resources/slipDamage", "slipDamage");
}

void EnemyManager::Initialize(Player* player) {

	// プレイヤーのアドレスを取得
	player_ = player;
	// 初期化処理
	// [ 生成インターバル ]
	spawnInterval_ = 3.0f;
	spawnTimer_ = 0.0f;// 0秒
	// [ 生成する場所の設定 ]
	spawnOffsetPos_ = { 0.0f,0.0,1.0f };// ｘとｙはズラさず、奥行きのみずらす
	// [ 初期速度 ]
	initialSpeed_ = 0.2f;
	// [ Model ] 
	obj_->SetModelData("zako");
	obj_->SetTexture("zako");
	obj_->Initialize({ 1.0f,1.0f,1.0f }, {0.0f,Math::Deg2Rad(90),Math::Deg2Rad(0)});
}

void EnemyManager::Update(Matrix4x4* m) {
	if (enemies_.size() < kMaxSpawnCount_) {
		Spawn(m);
	}
	// Enemy
	// [ 更新処理 ]
	for (const auto& enemy : enemies_) {
		enemy->Update(m);
	}
	// [ 削除 ]
	Delete();

	obj_->Update(m);
#ifdef _DEBUG
	ImGui::Begin("Manager");
	ImGui::DragFloat("interval", &spawnInterval_);
	Vector3 pos = obj_->GetTransform().translate;
	ImGui::DragFloat3("pos", &pos.x);
	SetPos(pos);
	ImGui::End();
#endif//_DEBUG
}

void EnemyManager::Draw() {
	obj_->Draw();
	for (const auto& enemy : enemies_) {
		enemy->Draw();
	}
}

void EnemyManager::Spawn(Matrix4x4* m) {

	spawnTimer_ += 1.0f / 60.0f;

	// 時間が達していなかったら処理しない
	if (spawnTimer_ < spawnInterval_) {
		return;
	}

	// ここから生成の準備処理
	
	// 攻撃判定の位置を更新(このモデルの回転を適用させる)
	// [ 計算できるようにMatを作成 ]
	Matrix4x4 w = Math::MakeAffineMatrix({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, spawnOffsetPos_);
	// [ 回転を取得 ]
	Matrix4x4 pW = Math::MakeAffineMatrix({ obj_->GetTransform().scale }, { obj_->GetTransform().rotate }, { obj_->GetTransform().translate });
	// [ 回転を適用 ]
	w = w * pW;
	// [ 位置情報を取得 == (向きを取得) ]
	Vector3 direction = { w.m[3][0],w.m[3][1], w.m[3][2] };

	// ここから生成処理
	// 生成
	std::unique_ptr<Enemy> newEnemy = std::make_unique<Enemy>(dxCommon_);
	// 初期化
	newEnemy->Initialize(direction, Math::Normalize(direction) * -initialSpeed_,player_);
	newEnemy->SetAriaLeftTop(&ariaLeftTop_);
	newEnemy->SetAriaSize(&ariaSize_);
	// 移行
	enemies_.push_back(std::move(newEnemy));

	// タイマーを初期化
	spawnTimer_ = 0.0f;
}

void EnemyManager::Delete() {
	std::erase_if(enemies_, [](const auto& enemy) {
		return enemy->GetIsFinished();
	});
}