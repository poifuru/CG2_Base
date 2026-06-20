#pragma once
#include <Windows.h>
#include <Wrl.h>
using namespace Microsoft::WRL;
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi1_6.h>
#include "CameraComponent.h"
#include "struct.h"
#include "MathFunction.h"
#include "InputManager.h"

class DebugCamera : public CameraComponent{
public:	//メンバ関数
	//コンストラクタ
	DebugCamera ();
	~DebugCamera () override;

	//初期化
	void Initialize (const EulerTransform& transform) override;

	//更新
	void Update () override;

	//ImGui
	void ImGui () override;

	bool GetTatchImGui () { return tatchImGui_; }
	void SetTatchImGui (bool flag) { tatchImGui_ = flag; }

private:	//メンバ変数
	//=====カメラの挙動に使う変数=======//
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
