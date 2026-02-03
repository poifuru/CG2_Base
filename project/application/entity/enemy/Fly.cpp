#include "Fly.h"
#include <cmath> // sinを使うために必要でやんす

Fly::Fly(DxCommon* dxCommon, LightManager* light, MapChip* mapchip)
	: BaseEnemy(dxCommon, light, mapchip) {
}

void Fly::Initialize() {
	// 初期位置を基準の高さとして保存しておくでやんす
	baseHeight_ = transform_.translate.y;
	hp_ = 1; // 体力の設定
	SetAABBSize({ 0.5f, 0.5f, 0.5f }); // 当たり判定のサイズ
}

void Fly::Update() {
	// 死亡演出などの共通処理は親クラスにお任せでやんす！
	BaseEnemy::Update();

	// 生きている時だけふよふよ動くでやんす
	if(!isDead_) {
		// --- ふよふよ移動の計算 ---
		floatingTimer_ += 0.05f; // 数値を大きくすると揺れが速くなるでやんす

		float amplitude = 0.5f; // 揺れ幅（どれくらい上下するか）

		// 基準の高さにサイン波を足すことで上下に揺らすでやんす
		transform_.translate.y = baseHeight_ + std::sin(floatingTimer_) * amplitude;

		// 左右にゆっくり動かしたい場合は velocity_.x をいじるといいでやんすね
		// transform_.translate.x += velocity_.x;

		// AABB（当たり判定）の更新を忘れずに！
		UpdateAABB();
	}
}