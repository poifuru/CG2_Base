#pragma once
#include "DxCommon.h"
#include "Model.h"
#include "Player.h"
#include "EnemyState.h"
#include "AttackCollider.h"
#include "EnemyBodyCollider.h"

class Enemy
{
public:
	Enemy(DxCommon* dxCommon) :dxCommon_(dxCommon) {};
	~Enemy() { delete state_; }
public:
	void Initialize(const Vector3& pos,const Vector3& velocity,Player*player);
	void Update(Matrix4x4* m);
	void Draw();
public:
	// 一般的な関数
	// [ Stateの状態遷移 ]
	void ChangeState(EnemyState* newState);
	// [ 攻撃を喰らった際の処理 ]
	void TakeDamage(float damage);
	void TakeSlipDamage();
	void TakeBossDamage();
	void Death() { isFinished_ = true; }
public:
	// 設定や取得の関数
	// [ 移動量 ]
	void SetMoveAmount(const Vector3& moveAmount) { moveAmount_ = moveAmount; }
	Vector3 GetMoveAmount()const { return moveAmount_; }
	// [ 攻撃の威力 ]
	void SetAttackLevel(const float& power) { attackLevel_ = power; }
	float GetAttackLevel()const { return attackLevel_; }
	// [ 生存フラグ ]
	void SetIsAlive(const bool& flag) { isAlive_ = flag; }
	bool GetIsAlive()const { return isAlive_; }
	// [ 終了フラグ ] 
	void SetIsFinished(const bool& flag) { isFinished_ = flag; }
	bool GetIsFinished()const { return isFinished_; }
	// [ 位置 ]
	//void SetPosition(const Vector3& pos) { obj_->SetTransform({obj_->GetTransform().scale,obj_->GetTransform().rotate,pos}); }
	Vector3 GetPosition()const { return obj_->GetTransform().translate; }
	// [ 回転 ]
	Vector3 GetRotation()const { return obj_->GetTransform().rotate; }
	void SetRotation(const Vector3& rot) { obj_->SetTransform({ obj_->GetTransform().scale,rot,obj_->GetTransform().translate }); }
	// [ ターゲット ]
	Player* GetTarget()const { return target_; }
	// [ ノックバックの方向 ]
	void SetKnockBackDirection(const Vector3& direction) { knockBackDirection_ = direction; }
	Vector3 GetKnockBackDirection()const { return knockBackDirection_; }
	// [ 歩くスピード ]
	void SetMoveSpeed(float speed) { moveSpeed_ = speed; }
	float GetMoveSpeed()const { return moveSpeed_; }
	// [ 敵の当たり判定 ]
	EnemyBodyCollider& GetBodyCollider() { return *bodyCollider_.get(); }
	void AddBodyHitType(uint32_t type);
	// [ 攻撃の当たり判定 ]
	AttackCollider& GetAttackCollider() { return *attackCollider_.get(); }
	// [ 当たり判定の設定 ]
	void EnableHitBox(bool enable,const Vector3& pos);
	void AddAttackHitType();
	void SetAttackRadius(const float& radius) { attackCollider_->SetRadius(radius); }
	void SetAttackRadiusForLevel();
	// [ エリア ]
	void SetAriaLeftTop(Vector3* ariaLeftTop) { ariaLeftTop_ = ariaLeftTop; }
	void SetAriaSize(Vector2* size) { ariaSize_ = size; }
	Vector2& GetAriaSize() { return *ariaSize_; }
	Vector3& GetAriaLeftTop() { return *ariaLeftTop_; }
	// [ エネミーがプレイヤーを向くかどうかのフラグ ]
	void SetIsViewPlayerFlag(float flag) { isViewPlayerFlag_ = flag; }
	// [ カラー関係 ]
	void SetColor(const Vector4& color) { obj_->SetColor(color); }
	void SetAlpha(float alpha) { obj_->SetColor({ 1.0f,1.0f,1.0f,alpha }); }
private:
	// 参照するもの(ポインタ型)
	DxCommon* dxCommon_ = nullptr;
	Player* target_ = nullptr;

	// 自身の持つパラメータ
	std::unique_ptr<Model>obj_;
	EnemyState* state_ = nullptr;
	Vector3 moveAmount_ = { 0.0f,0.0f,0.0f };
	float moveSpeed_ = 1.0f;
	// [ エネミーがプレイヤーを向くかどうかのフラグ ]
	bool isViewPlayerFlag_ = true;
	// [ 敵が死んだフラグ ]
	bool isAlive_ = true;
	// [ 敵をけしてもいいフラグ ]
	bool isFinished_ = false;
	// [ 攻撃の威力 ]
	float attackLevel_ = 0.0f;
	// [ ノックバックの方向 ]
	Vector3 knockBackDirection_ = { 0.0f,0.0f,0.0f };

	// 当たり判定(Collider)
	// [ 自身の体の当たり判定 ]
	std::unique_ptr<EnemyBodyCollider>bodyCollider_ = nullptr;
	// 見た目
	std::unique_ptr<Model>bodyColliderObj_;
	// [ 攻撃範囲の当たり判定 ]
	std::unique_ptr<AttackCollider>attackCollider_ = nullptr;
	// 見た目
	std::unique_ptr<Model>attackColliderObj_;
	// [ エリアのサイズ ]
	Vector3* ariaLeftTop_ = {};
	Vector2* ariaSize_ = {};
};

