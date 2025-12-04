#include "ThrowMinion.h"
#include "MathFunction.h" // Lerpなどを使うため
#include "../../Boss.h" // Bossクラスへのアクセス
#include <imgui.h>
#define _USE_MATH_DEFINES
#include <Math.h> 
#include <algorithm> 

// コンストラクタ
ThrowMinion::ThrowMinion(MagosuyaEngine* magosuya, Boss* boss) {
    magosuya_ = magosuya;
    boss_ = boss;

    // モデルデータ自体は一度だけロード
    magosuya_->LoadModelData("Resources/teapot", "teapot");
}

// デストラクタ
ThrowMinion::~ThrowMinion() {
}

// 初期化
void ThrowMinion::Initialize() {
    phase_ = AttackPhase::None;
    timer_ = 0;
    duration_ = 0;

    // 弾の状態を初期化し、各弾に専用のモデルを割り当てる (kNumProjectilesが増えたのでループ回数が増加)
    for (int i = 0; i < kNumProjectiles; ++i) {
        projectiles_[i].isActive = false;
        projectiles_[i].lifeTime = 0.0f;
        projectiles_[i].transform = { {0.3f,0.3f,0.3f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} }; // サイズを小さく設定

        // 弾ごとに新しいModelオブジェクトを生成・初期化
        projectiles_[i].model = std::make_unique<Model>(magosuya_);
        projectiles_[i].model->SetModelData("teapot");
        projectiles_[i].model->SetTexture("teapot");
        projectiles_[i].model->Initialize();

        // ★ 修正点1: 個別変数の初期化
        projectiles_[i].curveForce = 0.0f;
        projectiles_[i].currentCurveTimer = 0;
    }

    // 連続攻撃用パラメータの初期化
    totalThrows_ = 0;
    throwCount_ = 0;
    intervalFrames_ = 0;
    intervalTimer_ = 0;
    isCurving_ = false;
    // ★ 修正点1: 全体変数は削除
    // currentCurveForce_ = 0.0f;
    // curveTimer_ = 0;
}

// 攻撃開始
void ThrowMinion::StartAttack(Vector3 target, int numThrows, float intervalSeconds) {
    if (phase_ != AttackPhase::None) return;

    // パラメータの設定
    targetPos_ = target;
    totalThrows_ = numThrows;
    throwCount_ = 0;
    // 間隔（秒）をフレーム数に変換 (例: 1秒 = 60フレーム)
    intervalFrames_ = static_cast<int>(intervalSeconds * 60.0f);
    intervalTimer_ = 0;

    // 最初のフェーズへ移行
    phase_ = AttackPhase::Charge;
    timer_ = 0;
    duration_ = 60; // 溜め時間

    // 最初の弾はカーブから始める
    isCurving_ = true;
}

// 更新処理
void ThrowMinion::Update(Matrix4x4* m) {
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

    // 弾のモデルを個別に更新
    for (int i = 0; i < kNumProjectiles; ++i) {
        if (projectiles_[i].model) {
            projectiles_[i].model->Update(vp_);
        }
    }
}

// 溜めフェーズ
void ThrowMinion::UpdateCharge() {
    timer_++;

    if (timer_ >= duration_) {
        phase_ = AttackPhase::Shoot;
        timer_ = 0;
        // 最初の弾を発射
        float initialForce = isCurving_ ? kCurveForce_ : kShootForce_;
        EmitProjectiles(initialForce);

        throwCount_++;
        isCurving_ = !isCurving_; // 次の弾のためにフラグ反転
        intervalTimer_ = 0; // タイマーリセット
    }
}

// 発射・連続投球管理フェーズ
void ThrowMinion::UpdateShoot() {

    if (throwCount_ >= totalThrows_) {
        // すべての弾を発射し終わったらCooldownへ移行
        phase_ = AttackPhase::Cooldown;
        timer_ = 0;
        duration_ = 120;
        return;
    }

    intervalTimer_++;

    // 弾を発射するタイミングかチェック
    if (intervalTimer_ >= intervalFrames_) {

        // 現在のカーブ/シュートのタイプを設定して発射
        float force = isCurving_ ? kCurveForce_ : kShootForce_;
        EmitProjectiles(force);

        throwCount_++;
        isCurving_ = !isCurving_; // 次の弾のためにフラグを反転させる
        intervalTimer_ = 0; // タイマーリセット
    }
}

// 硬直フェーズ
void ThrowMinion::UpdateCooldown() {
    timer_++;
    if (timer_ >= duration_) {
        // すべて終了、通常状態へ戻る
        phase_ = AttackPhase::None;
    }
}

// 弾の生成と初速の設定 (曲がる力を設定)
void ThrowMinion::EmitProjectiles(float curveForce) {
    if (kNumProjectiles == 0) return;

    // 1. 非アクティブな弾を探す
    MinionProjectile* newProjectile = nullptr;
    for (int i = 0; i < kNumProjectiles; ++i) {
        if (!projectiles_[i].isActive) {
            newProjectile = &projectiles_[i];
            break;
        }
    }

    // 非アクティブな弾が見つからなかったら処理を中断（同時に存在する弾の最大数を超えた）
    if (!newProjectile) {
        return;
    }

    MinionProjectile& p = *newProjectile; // 見つかった非アクティブな弾を使用

    // 2. ボスの中心位置を取得
    Vector3 centerPos = boss_->GetPosition();

    // 3. ボスからの発射オフセットを計算 (ボスの左右どちらから出すか)
    Vector3 offset = { 0.0f, 0.0f, 0.0f };

    // ボスが -Z方向を向いていると仮定し、右方向を +X、左方向を -X とする
    if (curveForce < 0) { // kCurveForce_ (左に曲がる) の場合、右側から発射する (+X)
        offset = { -kThrowOffset_, 0.0f, 0.0f };
    } else {              // kShootForce_ (右に曲がる) の場合、左側から発射する (-X)
        offset = { kThrowOffset_, 0.0f, 0.0f };
    }

    // Y軸はボスの中心より少し上から出す
    centerPos.y += 1.0f;

    // 発射開始位置 = ボス中心 + オフセット
    Vector3 startPos = centerPos + offset;

    // ターゲット座標までの変位を計算 (Y軸は無視してXZ平面上の方向を計算)
    Vector3 displacement = targetPos_ - startPos;

    float initialVelY = 0.1f; // 軽く浮き上がらせる

    // 1. XZ平面上の距離と方向を計算
    float distXZ = std::sqrt(displacement.x * displacement.x + displacement.z * displacement.z);

    if (distXZ < 0.001f) {
        // 距離が近すぎる場合は処理を中断
        return;
    }

    // 2. XZ平面上の正規化された進行方向ベクトル
    float dirX = displacement.x / distXZ;
    float dirZ = displacement.z / distXZ;

    // 3. 初速を設定
    p.isActive = true;
    p.lifeTime = kMaxLifeTime;
    p.transform.translate = startPos;
    p.velocity = {
        dirX * kProjectileSpeed_,
        initialVelY,
        dirZ * kProjectileSpeed_
    };

    // ★ 修正点1: 弾ごとのカーブ力とタイマーを設定
    p.curveForce = curveForce;
    p.currentCurveTimer = 0;
}

// 弾一つ一つの移動・寿命の更新
void ThrowMinion::UpdateProjectiles() {

    // ★ 修正点1: 全体のカーブタイマーは使用しないため削除
    // curveTimer_++;

    for (int i = 0; i < kNumProjectiles; ++i) {
        MinionProjectile& p = projectiles_[i];
        if (p.isActive) {

            // 弾ごとのカーブタイマーをインクリメント
            p.currentCurveTimer++;

            // ★ 修正点2: ターゲットZ座標を越えたらカーブ力をゼロにする
            // ボスはZ軸+側、ターゲットはZ軸-側(-14.0f)にあると仮定
            if (p.transform.translate.z < targetPos_.z) {
                p.curveForce = 0.0f; // ターゲットを超えたらカーブ力をゼロにする
            }

            // 1. カーブの力を加える (カーブディレイ後)
            if (p.currentCurveTimer > kCurveDelay_) { // 弾ごとのタイマーを使用

                Vector3& vel = p.velocity;

                // 垂直ベクトル (Vz, -Vx) を計算
                float perpX = vel.z;
                float perpZ = -vel.x;

                // 垂直ベクトルを正規化
                float perpLength = std::sqrt(perpX * perpX + perpZ * perpZ);
                if (perpLength > 0.0001f) {
                    perpX /= perpLength;
                    perpZ /= perpLength;
                }

                // 弾ごとの p.curveForce を使用して速度を曲げる
                vel.x += perpX * p.curveForce; // 弾ごとのカーブ力を使用
                vel.z += perpZ * p.curveForce; // 弾ごとのカーブ力を使用

                // Y軸方向は軽く落下
                vel.y -= 0.005f;
            }

            // 2. 位置の更新 
            p.transform.translate.x += p.velocity.x;
            p.transform.translate.y += p.velocity.y;
            p.transform.translate.z += p.velocity.z;

            // 3. 寿命の更新と非アクティブ化
            p.lifeTime--;

            // スケールの更新（オプション: FullScreenAttackから引き継いだロジック。不要なら削除してください）
            const float kStartScale = 0.3f;
            const float kEndScale = 3.0f; // スケールを少し控えめに
            float t = 1.0f - (p.lifeTime / kMaxLifeTime);
            float currentScale = Lerp(kStartScale, kEndScale, t);
            p.transform.scale = { currentScale, currentScale, currentScale };

            if (p.lifeTime <= 0.0f) {
                p.isActive = false;
            }
        }
    }
}

// 描画処理
void ThrowMinion::Draw() {
    for (int i = 0; i < kNumProjectiles; ++i) {
        if (projectiles_[i].isActive && projectiles_[i].model) {
            // 1. 弾のトランスフォームを、その弾専用のモデルに設定
            projectiles_[i].model->SetTransform(projectiles_[i].transform);

            // 2. 専用のモデルを描画
            projectiles_[i].model->Draw();
        }
    }
}

// ImGuiコントロール
void ThrowMinion::ImGuiControl() {
#ifdef _DEBUG
    ImGui::Begin("Throw Minion Attack");

    // 攻撃テスト用ボタン
    static int testThrows = 3;
    static float testInterval = 0.5f;

    ImGui::InputInt("Throws##num", &testThrows);
    ImGui::InputFloat("Interval (sec)", &testInterval);

    if (ImGui::Button("Start Throw Minion Attack")) {
        // プレイヤーの位置を仮のターゲット (例: (0, 0, -14)) として渡す
        StartAttack({ 0.0f, 0.0f, -14.0f }, testThrows, testInterval);
    }

    // 現在の状態表示
    const char* stateStr = "None";
    switch (phase_) {
    case AttackPhase::Charge: stateStr = "Charge"; break;
    case AttackPhase::Shoot: stateStr = "Shoot (Throwing)"; break;
    case AttackPhase::Cooldown: stateStr = "Cooldown"; break;
    }
    ImGui::Text("State: %s", stateStr);
    ImGui::Text("Timer: %d / %d", timer_, duration_);

    ImGui::Separator();
    ImGui::Text("Throws: %d / %d", throwCount_, totalThrows_);
    ImGui::Text("Interval Timer: %d / %d", intervalTimer_, intervalFrames_);
    ImGui::Text("Next: %s", isCurving_ ? "Curve" : "Shoot");

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