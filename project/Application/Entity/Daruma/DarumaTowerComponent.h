#pragma once
#include "Component.h"
#include <vector>

enum class DarumaTowerState {
	Idle,         // 待機中（入力可能）
	KnockingOut,  // ブロック横弾き中
	Falling       // 上のブロック落下中
};

struct DarumaBlockInfo {
	GameObject* gameObject = nullptr;
	Vector3 targetPosition{ 0.0f, 0.0f, 0.0f };
	Vector3 velocity{ 0.0f, 0.0f, 0.0f };
	bool isKnockedOut = false;
};

class DarumaTowerComponent : public Component {
public:
	DarumaTowerComponent() = default;
	~DarumaTowerComponent() override = default;

	void Initialize() override;
	void Update() override;
	void ImGui() override;

	void Serialize(json& j) const override;
	void Deserialize(const json& j) override;

	const char* GetName() const override { return "DarumaTowerComponent"; }

	// タワーの生成
	void SpawnTower(int blockCount = 5);

	// 一番下のブロックを叩き出す
	void KnockOutBottomBlock();

private:
	std::vector<DarumaBlockInfo> blocks_; // 0番目が一番下
	DarumaTowerState state_ = DarumaTowerState::Idle;
	float knockOutTimer_ = 0.0f;
	float knockOutDuration_ = 0.2f; // 弾き出しアニメーションの時間（秒）
	float fallSpeed_ = 12.0f;        // 落下速度
	int initialBlockCount_ = 5;
	Vector3 blockScale_{ 2.0f, 1.0f, 2.0f };
	float blockHeight_ = 2.0f;     // ブロック自体の高さ
	float blockSpacing_ = 0.05f;   // ブロック同士の隙間・間隔
};
