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

	bool GetTatchImGui () { return tatchImGui_; }
	void SetTatchImGui (bool flag) { tatchImGui_ = flag; }

private:	//メンバ変数
	//ターゲットの位置
	Vector3* target_ = nullptr;
	//始点からターゲットまでの距離
	float distance_ = 0.0f;

	//カメラの正面に対しての前後左右に移動
	Vector3 forward_;	//カメラの前方ベクトル
	Vector3 right_;		//カメラの右ベクトル
	float speed_;		//移動スピード
	Vector3 move_;		//実際に移動に反映するベクトル

	//ドラッグで視点を変える
	float sensitivity_;	//ドラッグの感度
	float pitchOver_;	//上を向きすぎるのを防止する

	bool tatchImGui_ = false;	//ImGuiにカーソルが触れているか

	//ImGui識別用変数
	static inline int instanceNum_ = 0;

	//ポインタを借りる
	InputManager* input_ = nullptr;
};

