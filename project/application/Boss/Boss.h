#pragma once
#include "MagosuyaEngine.h"
#include "object/3d/Model.h"
#include "Attack/CenterStomp/CenterStomp.h"
#include "Attack/FullScreenAttack/FullScreenAttack.h"
#include "Attack/ThrowMinion/ThrowMinion.h"

class Player;

class Boss {
public:
	Boss(MagosuyaEngine* magosuya, Player* player);
	~Boss();

	void Initialize();
	void Update(Matrix4x4* m);
	void Draw();
	void ImGuiControl();

	bool IsAnyAttackActive() const;

	// Getter
	Transform& GetTransform() { return transform_; }
	Vector3& GetPosition() { return transform_.translate; }
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

	void UpdateMove();
	void BreathMove();
	void NormalMove();

	void WanderMove();
	void FollowMove();
	void EvadeMove();

	void UpdateMoveState();

private:
	MagosuyaEngine* magosuya_ = nullptr;
	Player* player_ = nullptr;

	// ボスのモデル
	std::unique_ptr<Model> model_ = nullptr;
	// 中央範囲攻撃
	std::unique_ptr <CenterStomp> centerStomp_ = nullptr;
	// 全画面攻撃
	std::unique_ptr <FullScreenAttack> fullScreenAttack_ = nullptr;
	// ブレス攻撃
	std::unique_ptr <ThrowMinion> throwMinion_ = nullptr;

	// ステートパターン
	MoveState moveState_ = MoveState::Wander;
	float followDistance_ = 20.0f; // プレイヤーを追尾開始する距離
	float evadeDistance_ = 3.0f;  // プレイヤーから離脱開始する距離
	int moveTimer_ = 0;           // ステート滞在時間
	int maxMoveTime_ = 180;       // 移動ステートを切り替えるフレーム数 (例: 3秒)

	Transform transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	float speed_ = 0.1f;

	// HP
	float maxHP_ = 100.0f;
	float hp_ = 100.0f;
};