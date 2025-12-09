#pragma once
#include "Model.h"
#include "CenterStomp.h"
#include "FullScreenAttack.h"
#include "Breath.h"
#include "DxCommon.h"
#include "InputManager.h"
#include "BossBodyCollider.h"
#include "MeshParticle.h"

class Player;

class Boss {
public:
	Boss(DxCommon* dxCommon, Player* player);
	~Boss();

	void Initialize();
	void Update(Matrix4x4* m);
	void Draw();
	void ImGuiControl();

	bool IsAnyAttackActive() const;
	void TakeDamage(float damage);

	// Getter
	Transform& GetTransform() { return transform_; }
	Vector3& GetPosition() { return transform_.translate; }
	Collider* GetBodyCollider() const { return bossBodyCollider_.get(); }
	std::vector<Collider*> GetAttackColliders();
	bool& GetIsAlive() { return isAlive_; }
	// Setter
	void SetTransform(Transform transform) { transform_ = transform; }
	void SetPosition(Vector3 position) { transform_.translate = position; }

private:
	enum class MoveState {
		Wander,   // 徘徊
		Follow,   // 追尾
		Evade,    // 離脱
		Attack,   // 攻撃中 (移動を停止するため)
	};

	enum class Rotate {
		right,
		left,
	};

	enum class WanderGoalType {
		Center,      // マップ中央
		MapEdge,     // マップ端
		LocalRandom  // 現在地付近のランダム
	};

	enum class AttackType {
		None,
		CenterStomp,
		FullScreenAttack,
		Breath
	};

	// 攻撃系の関数
	void UpdateAttack();
	AttackType SelectAttack(); // 攻撃を選択する関数
	void StartSelectedAttack(AttackType attackType); // 選択した攻撃を開始する関数

	// 行動系の関数
	void UpdateMove();
	void UpdateMoveState();
	void BreathMove();
	void NormalMove();
	void WanderMove();
	void FollowMove();
	void EvadeMove();
	void ClampPosition();

	// Hp関係の関数
	void UpdateHp();
	void DefineTheHpRange();

	// 歩くアニメーション
	void UpdateRotation();
	void UpdateAnimation();

	// 死亡したとき
	void UpdateDead();

private:
	std::unique_ptr<Model> model_ = nullptr;
	// 中央範囲攻撃
	std::unique_ptr <CenterStomp> centerStomp_ = nullptr;
	// 全画面攻撃
	std::unique_ptr <FullScreenAttack> fullScreenAttack_ = nullptr;
	// ブレス攻撃
	std::unique_ptr <Breath> Breath_ = nullptr;

	// --- ステートパターンと移動制御 ---
	MoveState moveState_ = MoveState::Wander;
	float followDistance_ = 20.0f; // プレイヤーを追尾開始する距離
	float evadeDistance_ = 3.0f;  // プレイヤーから離脱開始する距離
	int moveTimer_ = 0;           // ステート滞在時間
	int maxMoveTime_ = 180;       // 移動ステートを切り替えるフレーム数 (例: 3秒)

	Vector3 wanderTargetPos_ = { 0.0f, 0.0f, 0.0f }; // 徘徊の目標座標
	int wanderUpdateCount_ = 0;                      // 目標更新のカウンター
	WanderGoalType wanderGoalType_ = WanderGoalType::Center; // 現在の目標タイプ

	// --- 移動速度の調整係数 ---
	float wanderBaseSpeedFactor_ = 0.75f; // 基本の徘徊速度係数
	float wanderEdgeSpeedFactor_ = 1.25f; // マップ端へ向かう時の速度係数

	// --- ステート遷移の調整係数 ---
	float emergencyFollowFactor_ = 2.0f; // 遠すぎる場合に即座に追尾に切り替える距離係数
	float emergencyEvadeFactor_ = 0.5f;  // 近すぎる場合に即座に離脱に切り替える距離係数
	int wanderTimeFactor_ = 6;           // 自由徘徊の時間を基本移動時間の何倍にするか

	Transform transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f}, {0.0f,-1.0f,0.0f} };
	Rotate rotate_ = Rotate::left;
	float speed_ = 0.1f;

	// --- 攻撃AI用データ ---
	int attackCooldownTimer_ = 0; // 攻撃クールダウンタイマー
	int baseCooldownFrames_ = 300; // 基本の攻撃クールダウン (例: 5秒)
	float phase2SpeedFactor_ = 0.2f; // フェーズ2での攻撃速度倍率

	// 選択中の攻撃ステート
	AttackType currentAttack_ = AttackType::None;

	// HP
	float maxHP_ = 10000.0f;
	float hp_ = 10000.0f;
	bool isAlive_ = true;
	bool bossExtinction_ = false;

	// Collider
	std::unique_ptr<BossBodyCollider> bossBodyCollider_ = nullptr;
	std::unique_ptr<Model>bodyColliderObj_ = nullptr;
	float defaultRadius_ = 0.0f;
	float shadowRadius_ = 0.0f;

	//ポインタを借りる
	DxCommon* dxCommon_ = nullptr;
	Player* player_ = nullptr;
	InputManager* input_ = nullptr;
};