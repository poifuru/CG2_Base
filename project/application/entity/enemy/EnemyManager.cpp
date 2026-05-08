#include "EnemyManager.h"

void EnemyManager::Initialize() {
}

void EnemyManager::Update(float playerProgressZ) {
	// 出現予定リストを見てプレイヤーが一定距離進んでいたらスポーン
	while(currentSpawnIndex_ < spawnTimeline_.size()) {
		const auto& cmd = spawnTimeline_[currentSpawnIndex_];
		if(playerProgressZ >= cmd.triggerZ) {
			SpawnEnemy(cmd.enemyType, cmd.startPos);
			currentSpawnIndex_++;
		}
		else {
			break; // まだ出撃タイミングじゃないならループを抜ける
		}
	}
	// 2. 生きている敵の更新
	for(auto& enemy : enemies_) {
		if(enemy->IsActive()) { // アクティブな（画面にいる）敵だけ更新
			enemy->Update();

			// 画面後ろに過ぎ去ったり、HPが0になったら非アクティブに戻してプールに返却
			if(!enemy->IsActive()  /* or 画面外に出た判定*/) {
				enemy->SetIsActive(false);
			}
		}
	}
}

void EnemyManager::Draw() {
	for(auto& enemy : enemies_) {
		if(enemy->IsActive()) {
			enemy->Draw();
		}
	}
}

void EnemyManager::SpawnEnemy(int type, const Vector3& pos) {
}
