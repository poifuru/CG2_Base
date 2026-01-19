#pragma once
#include "MagosuyaEngine.h"
#include "SceneManager.h"

class Game {
public:
	Game();
	~Game();

	void Initialize();

	void Run();

private:
	MagosuyaEngine* magosuya_ = nullptr;
	SceneManager* sceneManager_ = nullptr;
};