#pragma once
#include "MagosuyaEngine.h"
#include "object/3d/Model.h"
#include "Attack/CenterStomp/CenterStomp.h"
#include "Attack/FullScreenAttack/FullScreenAttack.h"
#include "Attack/ThrowMinion/ThrowMinion.h"

class Boss {
public:
	Boss(MagosuyaEngine* magosuya);
	~Boss();

	void Initialize();
	void Update(Matrix4x4* m);
	void Draw();
	void ImGuiControl();

	bool IsAnyAttackActive() const;

	// Getter
	Transform& GetTransform() { return transform_; }
	Vector3& GetPosition() { return transform_.translate; }
	// Setter
	void SetTransform(Transform transform) { transform_ = transform; }
	void SetPosition(Vector3 position) { transform_.translate = position; }

private:
	void UpdateMove();
private:
	MagosuyaEngine* magosuya_ = nullptr;
	// ボスのモデル
	std::unique_ptr<Model> model_ = nullptr;
	// 中央範囲攻撃
	std::unique_ptr <CenterStomp> centerStomp_ = nullptr;
	// 全画面攻撃
	std::unique_ptr <FullScreenAttack> fullScreenAttack_ = nullptr;
	// カーブする球の攻撃
	std::unique_ptr <ThrowMinion> throwMinion_ = nullptr;

	Transform transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Vector3 speed_ = { 0.1f,0.1f,0.1f };

	// HP
	float maxHP_ = 100.0f;
	float hp_ = 100.0f;
};