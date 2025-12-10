#include "Enemy.h"
#include "MathFunction.h"
#include "imgui.h"

void Enemy::Initialize(const Vector3& pos, const Vector3& velocity, Player* player) {

	// 初期化処理
	// [ ターゲットの設定 ]
	target_ = player;
	
	// [ モデルの初期化 ]
	// -[ Enemy本体 ]-
	obj_ = std::make_unique<Model>(dxCommon_);
	obj_->SetModelData("zako");
	obj_->SetTexture("zako");
	obj_->Initialize();

	bodyColliderObj_ = std::make_unique<Model>(dxCommon_);
	bodyColliderObj_->SetModelData("zako");
	bodyColliderObj_->SetTexture("zako");
	bodyColliderObj_->Initialize();
	bodyColliderObj_->SetColor({0.0f,0.0f,0.0f,1.0f});

	attackColliderObj_ = std::make_unique<Model>(dxCommon_);
	attackColliderObj_->SetModelData("slipDamage");
	attackColliderObj_->SetTexture("slipDamage");
	attackColliderObj_->Initialize();

	// [ Colliderの設定 ]
	// -[ Body ]-
	bodyCollider_ = std::make_unique<EnemyBodyCollider>(this);// コンストラクタで初期化している
	// -[ Attack ]-
	attackCollider_ = std::make_unique<AttackCollider>();

	// [ 初期Stateの設定 ]
	state_ = new EnemyDecisionState();// 考えるState
	state_->SetEnemy(this);
	state_->Initialize();

	// [ 初期パラメータの設定 ]
	isAlive_ = true;// 生きているかのフラグ
	isFinished_ = false;// 終了のフラグ
	moveAmount_ = velocity;// 移動量の初期化
	obj_->SetTransform({ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {pos} });// 初期位置の設定
	Vector3 playerPos = bodyCollider_->GetWorldPosition();
	playerPos.y = -1.0f;
	bodyColliderObj_->SetTransform({ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {playerPos}});// 本体と位置合わせ
	EnableHitBox(false, playerPos);
}

void Enemy::Update(Matrix4x4* m) {
	// 毎フレームの初期化処理
	// [ 移動量をリセット ]
	//moveAmount_ = { 0.0f,0.0f,0.0f };

	

	// Stateの更新処理
	if (state_) {
		state_->Update();
	}

	// 移動の適用
	Vector3 newPos = obj_->GetTransform().translate + (moveAmount_ * moveSpeed_);

	// [ Levelによるサイズの変化 ]
	Vector3 scale = obj_->GetTransform().scale;
	scale = { 1.0f,1.0f,1.0f };
	if (attackLevel_ == 0.0f) {
		scale *= 1.0f;
	}
	if (attackLevel_ == 1.0f) {
		scale *= 1.3f;
	}
	if (attackLevel_ == 2.0f) {
		scale *= 1.8f;
	}
	if (attackLevel_ == 3.0f) {
		scale *= 2.3f;
	}

	Vector3 rotation = GetRotation();
	if (isViewPlayerFlag_ == true) {
		float toRotation = std::atan2(moveAmount_.x, moveAmount_.z);
		rotation.y = toRotation;
	}

	obj_->SetTransform({ scale,rotation,newPos });

	obj_->Update(m);

	// [ Colliderの更新 ]
	// -[ Body Collider ]-
	Vector3 playerPos = bodyCollider_->GetWorldPosition();
	playerPos.y = -1.0f;
	bodyColliderObj_->SetTransform({ {bodyCollider_->GetRadius(),0.1f,bodyCollider_->GetRadius()}, {0.0f,0.0f,0.0f}, {playerPos}});// 本体と位置合わせ
	bodyColliderObj_->Update(m);
	// -[ Attack Collider ]-
	Vector3 attackColliderPos = attackCollider_->GetWorldPosition();
	attackColliderObj_->SetTransform({ {attackCollider_->GetRadius() * 0.725f,0.1f,attackCollider_->GetRadius() * 0.725f}, {0.0f,0.0f,0.0f}, {attackColliderPos}});// 本体と位置合わせ
	attackColliderObj_->Update(m);

#ifdef _DEBUG
	//ImGui::Begin("Enemy");
	//if (attackCollider_->GetMyType() != COL_None) {
	//	// EnableHitBoxでTrueの場合値が変わることからなり立つ
	//	ImGui::Text("Enable : Treu");
	//}
	//ImGui::DragFloat("moveSpeed", &moveSpeed_);
	//ImGui::DragFloat3("pos", &newPos.x);
	//ImGui::DragFloat3("moveAmount", &moveAmount_.x);
	//ImGui::End();
	//obj_->ImGui("Enemy");
#endif//_DEBUG
}

void Enemy::Draw() {
	bodyColliderObj_->Draw();
	obj_->Draw();

	if (attackCollider_->GetMyType() != COL_None) {
		// EnableHitBoxでTrueの場合値が変わることからなり立つ
		attackColliderObj_->Draw();
	}
}

// ------------------------------
// Stateの状態遷移の処理
// ------------------------------

void Enemy::ChangeState(EnemyState*newState) {
	if (state_) {
		state_->Exit();

		delete state_;
		state_ = nullptr;
	}
	state_ = newState;
	state_->SetEnemy(this);
	state_->Initialize();
}

// -------------------------------
// Take Damage
// -------------------------------

void Enemy::TakeDamage(float damage) {
	// 1. 死亡状態、または無敵時間中の場合は処理をスキップ
	if (!isAlive_ /*|| IsInvulnerable()*/) {
		return;
	}

	// 2. 速度を設定する
	moveSpeed_ += damage;

	// 3. 状態遷移の判定
	// 被ダメージ State へ遷移
	ChangeState(new EnemyHurtState());

	// 無敵時間を設定する (HurtState側でリセットする)
	//SetInvulnerable(true);
}

void Enemy::TakeSlipDamage() {
	if (!isAlive_ /*|| IsInvulnerable()*/) {
		return;
	}

	// 3. 状態遷移の判定
	// 被ダメージ State へ遷移
	ChangeState(new EnemyPreExplosionState());
}

void Enemy::TakeBossDamage() {
	if (!isAlive_ /*|| IsInvulnerable()*/) {
		return;
	}

	ChangeState(new EnemyExplosionState());
}

// --------------------------------
// 当たり判定について
// --------------------------------

void Enemy::EnableHitBox(bool enable, const Vector3& pos) {
	if (enable) {

		attackCollider_->SetWorldPosition(pos);

		// 攻撃判定をアクティブ化
		attackCollider_->SetMyType(COL_Enemy_Attack);

		// 攻撃の威力による変化も追加
		AddAttackHitType();

		// 相手のマスクも設定
		attackCollider_->SetYourType(COL_Enemy | COL_Player | COL_Boss);
	}
	else {
		// 攻撃判定を非アクティブにする
		attackCollider_->SetMyType(COL_None);
		attackCollider_->SetYourType(COL_None);
	}
}

void Enemy::AddAttackHitType() {
	// 生きているなら
	if (isAlive_) {
		// 判定を追加
		if (attackLevel_ == 0.0f) {
			attackCollider_->SetMyType(attackCollider_->GetMyType() | COL_Enemy_Attack_Level0);
		}
		if (attackLevel_ == 1.0f) {
			attackCollider_->SetMyType(attackCollider_->GetMyType() | COL_Enemy_Attack_Level1);
		}
		if (attackLevel_ == 2.0f) {
			attackCollider_->SetMyType(attackCollider_->GetMyType() | COL_Enemy_Attack_Level2);
		}
		if (attackLevel_ == 3.0f) {
			attackCollider_->SetMyType(attackCollider_->GetMyType() | COL_Enemy_Attack_Level3);
		}
	}
	// 死んでいた場合
	else if (isAlive_ == false) {
		attackCollider_->SetMyType(attackCollider_->GetMyType() | COL_Enemy_SlipDamage);
	}
}

void Enemy::AddBodyHitType(uint32_t type) {
	if ((bodyCollider_->GetMyType() & type) != 0) {
		// 既に追加されていたら無視
		return;
	}
	bodyCollider_->SetMyType(bodyCollider_->GetMyType() | type);
}

// ---------------------------------
// Level による処理の追加
// ---------------------------------

void Enemy::SetAttackRadiusForLevel() {
	if (attackLevel_ == 0.0f) {
		SetAttackRadius(3.0f * 1.3f);
		obj_->SetColor({1.0f,1.0f,1.0f,1.0f});
	}
	if (attackLevel_ == 1.0f) {
		SetAttackRadius(4.0f * 1.3f);
		obj_->SetColor({ 0.8f,0.5f,0.5f,1.0f });
	}
	if (attackLevel_ == 2.0f) {
		SetAttackRadius(5.5f * 1.3f);
		obj_->SetColor({ 0.8f,0.2f,0.2f,1.0f });
	}
	if (attackLevel_ == 3.0f) {
		SetAttackRadius(7.0f  * 1.3f);
		obj_->SetColor({ 0.8f,0.0f,0.0f,1.0f });
	}
}