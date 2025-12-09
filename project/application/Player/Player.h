#pragma once
#include "MagosuyaEngine.h"
#include "PlayerState.h"
#include "Model.h"
#include "Collider.h"
#include "AttackCollider.h"
#include "PlayerBodyCollider.h"
#include "InputManager.h"

class Player
{
public:
	Player (InputManager* inputManager, DxCommon* dxCommon) : input_ (inputManager), dxCommon_(dxCommon) {};
	~Player();
public:
	void Initialize();
	void Update(Matrix4x4* m);
	void Draw();
public:
	void ChangeState(PlayerState* newState);
	// 攻撃の当たり判定の変更
	void EnableHitBox(bool enale, const Vector3& worldPos);
	// 進んでいる方向に向く処理
	void RotateToMoveDirection();
public:
	// 垂直速度の設定と取得
	void SetVerticalVelocity(float velocity) { verticalVelocity_ = velocity; }
	float GetVerticalVelocity() const { return verticalVelocity_; }
	// 地面判定の取得
	void SetIsOnGround(bool flag) { isOnGround_ = flag; }
	bool IsOnGround()const { return isOnGround_; }
	// スタミナの設定と取得
	void SetStamina(float cost) { stamina_ = cost; }
	float GetStamina() { return stamina_; }
	float GetStaminaRate() { return stamina_ / maxStamina_; }
	void DrainStamina(float amount) { stamina_ -= amount; }
	void BlockStaminaRecovery() { staminaRecoveryBlockers_++; }
	void UnblockStaminaRecovery() { if (staminaRecoveryBlockers_ > 0)staminaRecoveryBlockers_--; }
	// 移動速度倍率の設定と取得
	void SetSpeedMultiplier(float multiplier) { speedMultiplier_ = multiplier; }
	float GetSpeedMultiplier() { return speedMultiplier_; }
	// 状態チェック
	bool IsDead() const { return hp_ <= 0.0f; }
	bool IsInvulnerable() const { return isInvulnerable_; } // 無敵時間のチェック
	void SetInvulnerable(bool isInvulnerable) { isInvulnerable_ = isInvulnerable; }
	// AttackCollider
	// [ 取得 ]
	AttackCollider& GetAttackCollider() { return *attackCollider_.get(); }
	// [ Radius ]
	void SetAttackColliderRadius(float radius) { attackCollider_->SetRadius(radius); }
	// [ Typeの追加 ]
	void AddAttackColliderType(uint32_t type);

	// BodyCollider
	PlayerBodyCollider& GetPlayerBodyCollider() { return *playerCollider_.get(); }
	// 入力関係
	// [ 攻撃 ]
	bool IsAttack();
	// 位置の取得
	const Transform& GetTransform () { return obj_->GetTransform (); }
	Vector3 GetPosition() { return obj_->GetTransform().translate; }
	void TakeDamage(float damage);
	Vector3 GetForwardVector();
	void SetAlpha(float alpha) { obj_->SetColor({ 1.0f,1.0f,1.0f,alpha }); }
	// Quaternionの設定・取得
	//void SetPlayerQuaternion(const Quaternion& q) { obj_->worldTransform_.set_.Quaternion(q); }
	//Quaternion GetPlayerQuaternion() { return obj_->worldTransform_.get_.Quaternion(); }
	Vector3& Move() { return move_; }
	void SetIsViewAttack(bool flag) { isAttackViewFlag_ = flag; }
private:
	// 物理的な処理
	void ApplyPhysics();
	// スタミナ回復処理
	void UpdateStaminaRecovery();
public:
	InputManager* input_ = nullptr;
	DxCommon* dxCommon_ = nullptr;
private:
	std::unique_ptr<Model>obj_;
	PlayerState* state_ = nullptr;
	// キャラのCollider
	std::unique_ptr<PlayerBodyCollider>playerCollider_;
	std::unique_ptr<Model>playerColliderObj_;

	// 攻撃判定用のCollider
	std::unique_ptr<AttackCollider>attackCollider_;
	std::unique_ptr<Model>attackColliderObj_;
	bool isAttackViewFlag_ = false;

	// 無敵管理フラグ
	bool isInvulnerable_ = false;

	// HP
	float maxHP_ = 100.0f;
	float hp_ = 100.0f;

	// HPの可視化
	/*Sprite(緑)*/

	/*Sprite(赤)*/

	Vector3 move_ = { 0,0,0 };
	Vector3 direction_ = { 0.0f,0.0f,0.0f };
	// 一旦これは元のスピード
	float speed_ = 0.3f;
	// 移動速度倍率
	float speedMultiplier_ = 1.0f;
	// 旋回するスピード
	float rotateSpeed_ = 0.4f;

	// スタミナ
	float stamina_ = 100.0f;
	// マックススタミナ
	float maxStamina_ = 100.0f;
	// 1秒あたりの回復量
	float staminaRecoveryRate_ = 20.0f;
	// 回復を中断するかを確認するカウンター
	int staminaRecoveryBlockers_ = 0;

	// 垂直速度（Ｙ軸方向）
	float verticalVelocity_ = 0.0f;
	// 重力加速度
	float gravity_ = 0.098f;
	// 地面にいるかどうかのフラグ
	bool isOnGround_ = false;
};
