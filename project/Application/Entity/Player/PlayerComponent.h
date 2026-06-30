#pragma once
#include "Component.h"

// 前方宣言
class MainCameraComponent;

class PlayerComponent : public Component {
public:
	PlayerComponent() = default;
	~PlayerComponent() override = default;

	// Componentのライフサイクル関数
	void Initialize() override;
	void Update() override;
	void ImGui() override;

	// セーブ・ロード用の関数
	void Serialize(json& j) const override;
	void Deserialize(const json& j) override;

	//ゲッター
	const char* GetName() const override { return "PlayerComponent"; }

	// レティクル自動バインド用
	void ResolveReticle(const std::vector<std::unique_ptr<GameObject>>& gameObjects);

	
private:	// プライベート関数
	void Move();  // 移動処理
	void Shoot(); // 弾の発射処理

private:
	// プレイヤーのパラメータ
	float speed_ = 1.5f;
	Vector3 velocity_{};
	Vector3 acceleration_{};
	float cooltime_ = 0.0f;

	// 外部参照用ポインタ
	GameObject* reticleObject_ = nullptr;
};