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
	float speed_;
	float maxSpeed_; // 最高速度
	float attenuationRate_;	// 速度の減衰率(慣性)
	float brakeAttenuationRate_; // ブレーキ時の減衰（値が小さいほど急制動）
	Vector3 velocity_{};
	Vector3 acceleration_{};
	float cooltime_;		// 発射クールタイム
	float turnSpeed_;		// 左右への回転を補完するスピード
	float dirRatioZ_;		// 曲がるときにどのくらいの比率を掛けるか(前方)
	float dirRatioX_;		// 曲がるときにどのくらいの比率を掛けるか(左右)

	// ハープーンガンのパラメータ
	float harpoonSpeed_ = 120.0f;         // 弾速
	float harpoonHomingStrength_ = 0.02f; // 追尾力

	// 外部参照用ポインタ
	GameObject* reticleObject_ = nullptr;

	// パッド入力用
	float preTriggerR_ = 0.0f; // 前フレームのRT（右トリガー）の入力値
};