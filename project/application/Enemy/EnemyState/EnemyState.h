#pragma once
#include "struct.h"
class Enemy; // 前方宣言

// Enemy Attack Levelについて
// Lv 1, 2.0
// Lv 2, 3.0
// Lv 3, 4.5
// Lv 4, 6.0

// ボス用 State 抽象クラス
class EnemyState
{
public:
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Exit() = 0;
	void SetEnemy(Enemy* Enemy) { enemy_ = Enemy; }
protected:
	Enemy* enemy_ = nullptr;
};

// 1. 待機・意思決定を行う State(Brain)
class EnemyDecisionState
	:public EnemyState
{
public:
	void Initialize() override;
	void Update() override;
	void Exit() override;
private:
	// 行動の評価基準となる定数
	float decisionTimer_ = 0.0f;// 意思決定タイマー
	float maxDecisionTime_ = 1.0f;// 意思決定終了の時間
};

// 2. Playerを探す State
class EnemySearchPlayerState
	:public EnemyState
{
public:
	void Initialize() override;
	void Update() override;
	void Exit() override;
private:
	float searchTimer_ = 0.0f;
	float maxSearchTime_ = 3.0f;// 索敵を辞める時間
	float viewDistance_ = 20.0f;// 視認距離
};

// 3. プレイヤーを見つけた時のState(飛んで見つけたことが分かるようにしたい)
class EnemyFindPlayerState
	:public EnemyState
{
public:
	void Initialize()override;
	void Update()override;
	void Exit()override;
private:
	float findTimer_ = 0.0f;
	float maxFindTime_ = 1.0f;
};

// 4. プレイヤーに近づく State
class EnemyChaseState
	:public EnemyState
{
public:
	void Initialize() override;
	void Update() override;
	void Exit() override;
private:
	float chaseSpeedRate_ = 5.0f;          // 1秒当たりのボスの移動速度
	float startExplosionDist_ = 4.0f;   // プレイヤーとの距離がこれ以下になったら爆発を開始する
};

// 5. 爆発するまで最終接近と警告の State
class EnemyPreExplosionState
	:public EnemyState
{
public:
	void Initialize() override;
	void Update() override;
	void Exit() override;
private:
	float preExplosionTimer_ = 0.0f;// 爆発までのタイマー
	float maxPreExplosionTime_ = 2.0f; // 爆発へ遷移する時間(逃がすために)
	float warningRadius_ = 6.0f;// 爆発する警告範囲
	float chaseSpeedRate_ = 1.7f;// プレイヤーを追いかける速度
};

// 6. 爆発の State
class EnemyExplosionState
	:public EnemyState
{
public:
	void Initialize()override;
	void Update()override;
	void Exit()override;
private:
	float explosionTimer_ = 0.0f;
	float maxExplosionTime_ = 1.0f;
};

// 7. ダメージを受けた際の State(モンストみたいに初速をうけ、跳ね返りまくる)
class EnemyHurtState
	:public EnemyState
{
public:
	void Initialize() override;
	void Update() override;
	void Exit() override;
private:
	float initialSpeed_ = 1.0f;// ダメージを受けた際の初速度(攻撃の威力によって変更)
	Vector3 moveAmount_;
	float decelerationRate_ = 0.4f;// １秒あたりの減速速度
	float minSpeed_ = 0.1f; // 速度がこれ以下になったらExplosionに移行
	float e_ = 0.95f;// 反発係数
	float boundCounter_ = 0.0f;// 壁に当たった回数
	float attackLevel_ = 2.0f;// 当たった回数で威力が変化
};

// 8. 死亡の State(スリップダメージも含む)
class EnemyDeathState
	:public EnemyState
{
public:
	void Initialize() override;
	void Update() override;
	void Exit() override;
private:
	float deathTimer_ = 0.0f;           // 演出の経過時間
	float maxDeathDuration_ = 3.0f; // 演出の持続時間 (3.0秒)
};