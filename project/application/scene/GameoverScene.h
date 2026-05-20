//#pragma once
//#include "BaseScene.h"
//#include <memory>
//#include "Sprite.h"
//#include "Model.h"
//
//class GameoverScene : public BaseScene {
//public:		//メンバ関数
//	GameoverScene ();
//	~GameoverScene () override;
//
//	void Initialize (CameraOrganizer* camera, InputManager* inputManager, DxCommon* dxCommon) override;
//	void Update () override;
//	void Draw () override;
//	void StopToResources() override;
//	
//private:	//メンバ変数
//	std::unique_ptr<LightManager> lightManager_ = nullptr;
//	std::unique_ptr<Sprite> gameover_ = nullptr;
//	std::unique_ptr<Model> skydome_ = nullptr;
//};