#include "FullScreenAttack.h"
#include "MathFunction.h"
#include "Boss.h"
#include "ModelManager.h"
#include <imgui.h>
#define _USE_MATH_DEFINES
#include <Math.h>
#include <algorithm>

FullScreenAttack::FullScreenAttack(DxCommon* dxCommon, Boss* boss) {
	dxCommon_ = dxCommon;
	boss_ = boss;

	// ★ モデルデータ自体は一度だけロードする（モデルのインスタンスはInitializeで生成）
	ModelManager::GetInstance()->LoadModelData ("Resources/teapot", "teapot");
}

FullScreenAttack::~FullScreenAttack() {
}

void FullScreenAttack::Initialize() {
	phase_ = AttackPhase::None;
	timer_ = 0;
	duration_ = 0;

	// 弾の状態を初期化し、各弾に専用のモデルを割り当てる
	for (int i = 0; i < kNumProjectiles; ++i) {
		projectiles_[i].isActive = false;
		projectiles_[i].lifeTime = 0.0f;
		projectiles_[i].transform = { {0.3f,0.3f,0.3f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} }; // サイズを小さく設定

		// ★ 弾ごとに新しいModelオブジェクトを生成・初期化する
		projectiles_[i].model = std::make_unique<Model>(dxCommon_);
		projectiles_[i].model->SetModelData("teapot");
		projectiles_[i].model->SetTexture("teapot");
		projectiles_[i].model->Initialize();

		projectiles_[i].collider = std::make_unique<BossProjectileCollider>(
			&projectiles_[i].transform.translate,
			5.0f,
			COL_Boss_Attack_FullScreenBullet
		);
	}
}

void FullScreenAttack::StartAttack() {
	if (phase_ != AttackPhase::None) return;

	phase_ = AttackPhase::Charge;
	timer_ = 0;
	duration_ = 60; // 溜め時間: 60フレーム（1秒）
}

void FullScreenAttack::Update(Matrix4x4* m) {
	vp_ = m;

	// 状態遷移マシンの実行
	switch (phase_) {
	case AttackPhase::Charge:
		UpdateCharge();
		break;
	case AttackPhase::Shoot:
		UpdateShoot();
		break;
	case AttackPhase::Cooldown:
		UpdateCooldown();
		break;
	case AttackPhase::None:
	default:
		break;
	}

	// 弾の更新処理はフェーズに関わらず実行
	UpdateProjectiles();

	// ★ 16個のモデルすべてを個別に更新する
	for (int i = 0; i < kNumProjectiles; ++i) {
		if (projectiles_[i].model) {
			projectiles_[i].model->Update(vp_);
		}
	}
}

// 溜めフェーズ
void FullScreenAttack::UpdateCharge() {
	timer_++;

	// ここでボスが光るなどの予兆処理

	if (timer_ >= duration_) {
		phase_ = AttackPhase::Shoot;
		timer_ = 0;
		duration_ = 30; // 弾が飛び続ける時間を設定しても良いが、今回は発射後すぐにCooldownへ

		// 弾を発射
		EmitProjectiles();
	}
}

// 発射フェーズ (ここでは瞬間的に次のフェーズへ移行)
void FullScreenAttack::UpdateShoot() {
	// 発射処理は EmitProjectiles() で一度だけ行うため、ここでは主にアニメーションなどの処理

	phase_ = AttackPhase::Cooldown;
	timer_ = 0;
	duration_ = 120; // 硬直時間: 2秒
}

// 硬直フェーズ
void FullScreenAttack::UpdateCooldown() {
	timer_++;
	if (timer_ >= duration_) {
		// すべて終了、通常状態へ戻る
		phase_ = AttackPhase::None;
	}
}

// 弾の生成と初速の設定
void FullScreenAttack::EmitProjectiles() {
	// 発射の中心座標はボスの現在位置
	Vector3 centerPos = boss_->GetPosition();

	// 360度を kNumProjectiles で分割して角度を計算
	const float kAngleStep = 360.0f / kNumProjectiles;

	for (int i = 0; i < kNumProjectiles; ++i) {
		float angle = kAngleStep * i;
		// 角度をラジアンに変換
		float radian = angle * (float)M_PI / 180.0f;

		// 進行方向の計算 (XZ平面)
		// 角度0度を+X軸方向とするために、Xにcos、Zにsinを適用
		float velX = std::cos(radian);
		float velZ = std::sin(radian);

		// 弾の初期化
		projectiles_[i].isActive = true;
		projectiles_[i].lifeTime = kMaxLifeTime;
		projectiles_[i].transform.translate = centerPos;

		// 速度を設定 (Y軸方向は動かさない)
		projectiles_[i].velocity = { velX * kProjectileSpeed_, 0.0f, velZ * kProjectileSpeed_ };
	}
}

// 弾一つ一つの移動・寿命の更新
void FullScreenAttack::UpdateProjectiles() {
	// 最大初期スケール
	const float kStartScale = 0.3f;
	// 最大最終スケール (例: 9.0fまで大きくなるように設定)
	const float kEndScale = 9.0f;

	for (int i = 0; i < kNumProjectiles; ++i) {
		if (projectiles_[i].isActive) {

			// 1. 移動 (変更なし)
			projectiles_[i].transform.translate.x += projectiles_[i].velocity.x;
			projectiles_[i].transform.translate.y += projectiles_[i].velocity.y;
			projectiles_[i].transform.translate.z += projectiles_[i].velocity.z;

			// 2. 寿命を減らす (変更なし)
			projectiles_[i].lifeTime--;

			// 3. スケールの更新（弾が飛ぶほどスケールを大きくする）
			// lifeTime の逆数を正規化 (0.0f から 1.0f へ) して Lerp に使う
			// 0.0f (発射直後) -> 1.0f (寿命切れ直前)
			float t = 1.0f - (projectiles_[i].lifeTime / kMaxLifeTime);

			// t を使ってスケールを線形補間 (Lerp)
			float currentScale = Math::Lerp(kStartScale, kEndScale, t);

			// Transformに適用
			projectiles_[i].transform.scale = { currentScale, currentScale, currentScale };

			// 4. 寿命が尽きたら非アクティブ化 (変更なし)
			if (projectiles_[i].lifeTime <= 0.0f) {
				projectiles_[i].isActive = false;
			}

			// TODO: プレイヤーとの衝突判定もここで行う
		}
	}
}

void FullScreenAttack::Draw() {
	// インスタンス描画ではなく、ループで個別に描画する

	for (int i = 0; i < kNumProjectiles; ++i) {
		// ★ 個別のモデルインスタンスに対して処理を行う
		if (projectiles_[i].isActive && projectiles_[i].model) {
			// 1. 弾のトランスフォームを、その弾専用のモデルに設定
			projectiles_[i].model->SetTransform(projectiles_[i].transform);

			// 2. 専用のモデルを描画
			projectiles_[i].model->Draw();
		}
	}
}

void FullScreenAttack::ImGuiControl() {
#ifdef USEIMGUI
	ImGui::Begin("FullScreen Attack");

	// 攻撃テスト用ボタン
	if (ImGui::Button("Start Full Screen Attack")) {
		StartAttack();
	}

	// 現在の状態表示
	const char* stateStr = "None";
	switch (phase_) {
	case AttackPhase::Charge: stateStr = "Charge"; break;
	case AttackPhase::Shoot: stateStr = "Shoot"; break;
	case AttackPhase::Cooldown: stateStr = "Cooldown"; break;
	}
	ImGui::Text("State: %s", stateStr);
	ImGui::Text("Timer: %d / %d", timer_, duration_);

	// アクティブな弾の数を表示
	int activeCount = 0;
	for (int i = 0; i < kNumProjectiles; ++i) {
		if (projectiles_[i].isActive) {
			activeCount++;
		}
	}
	ImGui::Text("Active Projectiles: %d / %d", activeCount, kNumProjectiles);

	ImGui::End();
#endif
}

std::vector<Collider*> FullScreenAttack::GetColliders() {
	std::vector<Collider*> colliders;
	for (int i = 0; i < kNumProjectiles; ++i) {
		// 弾がアクティブ、かつColliderが存在し、アクティブな場合にリストに追加
		if (projectiles_[i].isActive && projectiles_[i].collider) {
			colliders.push_back(projectiles_[i].collider.get());
		}
	}
	return colliders;
}