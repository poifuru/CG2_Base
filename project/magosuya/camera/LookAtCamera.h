#pragma once
#include "CameraComponent.h"
#include "InputManager.h"

class LookAtCamera : public CameraComponent {
public:	//メンバ関数
	//コンストラクタ
	LookAtCamera (InputManager* inputManager);
	~LookAtCamera () override;

	//初期化
	void Initialize (const Transform& transform) override;

	//更新
	void Update () override;

	//ImGui
	void ImGui () override;

	void SetPosition (const Vector3& pos) { camera_.transform.translate = pos; }
	void SetTarget (const Vector3& targetPos) { target_ = targetPos; }
	bool GetTouchImGui () const { return touchImGui_; }
	void SetTouchImGui (bool flag) { touchImGui_ = flag; }

private:	//メンバ変数
	//ターゲットの位置
	Vector3 target_ = {};
	//始点からターゲットまでの距離
	float distance_ = 0.0f;
	//上向きベクトル(固定)
	const Vector3 upVector_ = { 0.0f, 1.0f, 0.0f };

	//ドラッグで視点を変える
	float sensitivity_ = 0.0f;	//ドラッグの感度
	float pitchOver_ = 0.0f;	//上を向きすぎるのを防止する

	bool touchImGui_ = false;	//ImGuiにカーソルが触れているか

	//ImGui識別用変数
	static inline int instanceNum_ = 0;

	//ポインタを借りる
	InputManager* input_ = nullptr;
};

