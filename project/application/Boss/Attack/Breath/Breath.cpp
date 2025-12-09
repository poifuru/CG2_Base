#include "Breath.h"
#include "MathFunction.h" 
#include "Boss.h" 
#include "ModelManager.h"
#include <imgui.h>
#define _USE_MATH_DEFINES
#include <Math.h> 
#include <algorithm> 

// コンストラクタ
Breath::Breath(DxCommon* dxCommon, Boss* boss) {
    dxCommon_ = dxCommon;
    boss_ = boss;

    //ModelManager::GetInstance()->LoadModelData ("Resources/teapot", "teapot");
}

// デストラクタ
Breath::~Breath() {
}

// 初期化
void Breath::Initialize() {
    phase_ = AttackPhase::None;
    timer_ = 0;
    duration_ = 0;

    for (int i = 0; i < kNumProjectiles; ++i) {
        projectiles_[i].isActive = false;
        projectiles_[i].lifeTime = 0.0f;
        // スケールを0.3fで固定
        projectiles_[i].transform = { {0.1f,0.1f,0.1f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };

        projectiles_[i].model = std::make_unique<Model>(dxCommon_);
        projectiles_[i].model->SetModelData("fireBall");
        projectiles_[i].model->SetTexture("fireBall");
        projectiles_[i].model->Initialize();

        // カーブ関連の初期化を削除/維持 (構造体の初期値 0.0f/0 を利用)
        projectiles_[i].curveForce = 0.0f;
        projectiles_[i].currentCurveTimer = 0;
        projectiles_[i].isCurvingZ = false;

        projectiles_[i].collider = std::make_unique<BossProjectileCollider>(
            &projectiles_[i].transform.translate,
            3.0f,
            COL_Boss_Attack_Breath
        );
    }

    totalThrows_ = 0;
    throwCount_ = 0;
    intervalFrames_ = 0;
    intervalTimer_ = 0;
    // isCurving_ の設定を削除
}

// 攻撃開始
void Breath::StartAttack(int numThrows, float intervalSeconds) {
    if (phase_ != AttackPhase::None) return;

   totalThrows_ = numThrows;
    throwCount_ = 0;
    intervalFrames_ = static_cast<int>(intervalSeconds * 60.0f);
    intervalTimer_ = 0;

    phase_ = AttackPhase::Charge;
    timer_ = 0;
    duration_ = 60;

    // isCurving_ の設定を削除
}

// 更新処理
void Breath::Update(Matrix4x4* m, Vector3 target) {
    vp_ = m;
    targetPos_ = target;

    switch (phase_) {
    case AttackPhase::Charge: UpdateCharge(); break;
    case AttackPhase::Shoot: UpdateShoot(); break;
    case AttackPhase::Cooldown: UpdateCooldown(); break;
    case AttackPhase::None: default: break;
    }

    UpdateProjectiles();

    for (int i = 0; i < kNumProjectiles; ++i) {
        if (projectiles_[i].isActive && projectiles_[i].model) {
            projectiles_[i].model->Update(vp_);
        }
    }
}

// 溜めフェーズ
void Breath::UpdateCharge() {
    timer_++;
    if (timer_ >= duration_) {
        phase_ = AttackPhase::Shoot;
        timer_ = 0;
        // isCurving_ の反転を削除
        intervalTimer_ = 0;
    }
}

// 発射・連続投球管理フェーズ
void Breath::UpdateShoot() {
    if (throwCount_ >= totalThrows_) {
        phase_ = AttackPhase::Cooldown;
        timer_ = 0;
        duration_ = 120;
        return;
    }

    intervalTimer_++;
    if (intervalTimer_ >= intervalFrames_) {
        // 次の弾を発射
        EmitProjectiles();
        throwCount_++;
        // isCurving_ の反転を削除
        intervalTimer_ = 0;
    }
}

// 硬直フェーズ
void Breath::UpdateCooldown() {
    timer_++;
    if (timer_ >= duration_) {
        phase_ = AttackPhase::None;
    }
}

// 弾の生成と初速の設定
void Breath::EmitProjectiles() {
    if (kNumProjectiles == 0) return;

    MinionProjectile* newProjectile = nullptr;
    for (int i = 0; i < kNumProjectiles; ++i) {
        if (!projectiles_[i].isActive) {
            newProjectile = &projectiles_[i];
            break;
        }
    }

    if (!newProjectile) { return; }
    MinionProjectile& p = *newProjectile;

    // 1. ボスの中心位置と発射位置の決定
    Vector3 centerPos = boss_->GetPosition();

    Vector3 offset = { 0.0f, 0.0f, 0.0f };

    Vector3 startPos = centerPos + offset;

    // 2. 初速ベクトルの計算 (ターゲットにまっすぐ向かう水平方向)
    Vector3 displacement = targetPos_ - startPos;
    displacement.y = 0.0f; // Y軸は無視 (水平初速のみ)

    float dist2D = std::sqrt(displacement.x * displacement.x + displacement.z * displacement.z);

    if (dist2D < 0.001f) {
        return;
    }

    // 3. 初速を設定
    float dirX = displacement.x / dist2D;
    float dirZ = displacement.z / dist2D;

    // ブレ
    float randX = ((float)rand() / RAND_MAX - 0.5f) * 0.15f;
    float randZ = ((float)rand() / RAND_MAX - 0.5f) * 0.15f;

    p.isActive = true;
    p.lifeTime = kMaxLifeTime;
    p.transform.translate = startPos;
    p.velocity = {
        dirX * kProjectileSpeed_ + randX,
        0.0f, // Y軸速度は 0
        dirZ * kProjectileSpeed_ + randZ
    };

    p.curveForce = 0.0f; // 念のためゼロ設定を維持
    p.currentCurveTimer = 0;
    p.model->SetTransform(p.transform);
}

// 弾一つ一つの移動・寿命の更新 (純粋な直線移動のみ)
void Breath::UpdateProjectiles() {

    for (int i = 0; i < kNumProjectiles; ++i) {
        MinionProjectile& p = projectiles_[i];
        if (p.isActive) {
            // 1. 位置の更新 
            p.transform.translate.x += p.velocity.x;
            p.transform.translate.y += p.velocity.y;
            p.transform.translate.z += p.velocity.z;

            // ★ 向きの更新 (モデルの正面が -Z のため、π (M_PI) を加算して 180 度回転させる)
            float angleY = std::atan2(p.velocity.x, p.velocity.z);
            p.transform.rotate.y = angleY + (float)M_PI;

            // 2. 寿命の更新と非アクティブ化
            p.lifeTime--;

            // スケールを固定 (kFixedScale で固定)
            const float kFixedScale = 1.0f;
            p.transform.scale = { kFixedScale, kFixedScale, kFixedScale };

            if (p.transform.translate.x < -45.0f || p.transform.translate.x > 45.0f ||
                p.transform.translate.z < -45.0f || p.transform.translate.z > 45.0f) {
                p.isActive = false;
                continue; // 非アクティブ化されたら次の弾へ
            }

            if (p.lifeTime <= 0.0f) {
                p.isActive = false;
            }
        }
    }
}

// 描画処理
void Breath::Draw() {
    for (int i = 0; i < kNumProjectiles; ++i) {
        if (projectiles_[i].isActive && projectiles_[i].model) {
            projectiles_[i].model->SetTransform(projectiles_[i].transform);
            projectiles_[i].model->Draw();
        }
    }
}

// ImGuiコントロール
void Breath::ImGuiControl() {
#ifdef USEIMGUI
    ImGui::Begin("Throw Minion Attack");
    static int testThrows = 150;
    static float testInterval = 0.001f;

    ImGui::InputInt("Throws##num", &testThrows);
    ImGui::InputFloat("Interval (sec)", &testInterval);

    if (ImGui::Button("Start Throw Minion Attack")) {
        // ターゲット座標 (0.0f, 0.0f, -10.0f) を渡す
        StartAttack(testThrows, testInterval);
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

    ImGui::End();
#endif
}

std::vector<Collider*> Breath::GetColliders() {
    std::vector<Collider*> colliders;
    for (int i = 0; i < kNumProjectiles; ++i) {
        // 弾がアクティブ、かつColliderが存在し、アクティブな場合にリストに追加
        if (projectiles_[i].isActive && projectiles_[i].collider) {
            colliders.push_back(projectiles_[i].collider.get());
        }
    }
    return colliders;
}