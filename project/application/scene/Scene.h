#pragma once
#include <memory>
#include "CameraOrganizer.h"
#include "InputManager.h"
#include "Audio.h"

//シーンのラベル
enum class SceneLabel {
	Title,
	Play,
	Clear,
	Gameover,

	//フェード
	Fade,
};

class MagosuyaEngine;

class Scene {
public:		//メンバ関数
	virtual ~Scene () = default;

	virtual void Initialize () = 0;
	virtual void Update () = 0;
	virtual void Draw () = 0;

	bool GetIsFinish () { return isFinish_; }
	void SetNowScene (SceneLabel scene) { nowScene_ = scene; }
	SceneLabel GetNowScene () { return nowScene_; }
	SceneLabel GetNextScene () { return nextScene_; }

protected:	//メンバ変数
	bool isFinish_ = false;
	//今のシーン
	SceneLabel nowScene_ = SceneLabel::Title;
	//次に行きたいシーンを持たせる
	SceneLabel nextScene_ = SceneLabel::Title;

	Audio audio_;
	SoundData bgmHandle_;
	SoundData clickHandle_;

	//ポインタを借りる
	CameraOrganizer* camera_ = nullptr;
	InputManager* input_ = nullptr;
};
