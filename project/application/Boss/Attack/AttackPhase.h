#pragma once

enum class AttackPhase {
    None,       // 何もしていない
    Charge,     // 予兆・溜め
    Shoot,      // 弾発射/攻撃実行
    Cooldown    // 硬直
};