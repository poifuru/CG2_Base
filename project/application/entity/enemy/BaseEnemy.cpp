#include "BaseEnemy.h"

BaseEnemy::BaseEnemy(DxCommon* dxCommon, LightManager* light, MapChip* mapchip) {
	model_ = std::make_unique<Model>(dxCommon, light);
	mapchip_ = mapchip;
}

BaseEnemy::~BaseEnemy() {

}

void BaseEnemy::Initialize() {

}

void BaseEnemy::Update() {
	// もし死亡演出中なら透明度を下げる
	if(isDead_) {
		alpha_ -= kFadeSpeed;
		if(alpha_ < 0.0f) alpha_ = 0.0f;

		// モデルに透明度を反映
		// SetColorの第4引数がアルファ値（A）でやんす！
		model_->SetColor({ 1.0f, 1.0f, 1.0f, alpha_ });
		return; // 死亡演出中は移動などの処理はさせないでやんす
	}

	// 無敵タイマーが動いていたら減らす
	if(invincibleTimer_ > 0) {
		invincibleTimer_--;

		// 無敵中はモデルを点滅させると分かりやすいでやんす！
		if(invincibleTimer_ % 10 < 5) {
			model_->SetColor({ 1.0f, 0.0f, 0.0f, 0.4f }); // 透けさせる
		}
		else {
			model_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f }); // 通常
		}
	}
	else {
		// 無敵が終わったら色を完全に戻す
		model_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	}
}

void BaseEnemy::Draw() {
	if(alpha_ <= 0.0f) return; 
	model_->Draw();
}

void BaseEnemy::ImGui() {
	model_->ImGui("enemy");
}

void BaseEnemy::OnHit(int damage, const Vector3& playerPos) {
	if(invincibleTimer_ > 0) return;

	hp_ -= damage;
	// ダメージを受けたら無敵タイマーをセット
	if(hp_ <= 0) {
		hp_ = 0;
		isDead_ = true; // 死亡演出スタート！
		// 倒した瞬間に少し上に跳ねさせると「やられた感」が出るでやんす
		velocity_.y = 0.2f;
		velocity_.x = (transform_.translate.x > playerPos.x) ? 0.1f : -0.1f;
	}
	else {
		invincibleTimer_ = kInvincibleTime;

		// --- ノックバック処理 ---
		float knockbackPower = 0.1f;
		float upPower = 0.15f; // 少し上に浮かすとそれっぽい！

		// プレイヤーと逆方向に飛ばす
		if(transform_.translate.x > playerPos.x) {
			velocity_.x = knockbackPower;
		}
		else {
			velocity_.x = -knockbackPower;
		}
		velocity_.y = upPower; // ちょっと跳ねる

		// 被弾した瞬間に赤くするなどの演出
		model_->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });
	}
}