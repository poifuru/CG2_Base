#pragma once
#include "Scene.h"
#include <memory>
#include "Sprite.h"
#include "Model.h"

class PlayScene : public Scene {
public:		//メンバ関数
	PlayScene (MagosuyaEngine* magosuya);
	~PlayScene () override;

	void Initialize () override;
	void Update () override;
	void Draw () override;

private:	//メンバ変数
	std::unique_ptr<CameraData> camera_ = nullptr;
	std::unique_ptr<Player>player_ = nullptr;
	std::unique_ptr<Boss> boss_ = nullptr;
};
