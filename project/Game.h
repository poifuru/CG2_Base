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
	std::unique_ptr<SceneManager> sceneManager_ = nullptr;
};