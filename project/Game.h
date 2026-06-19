#pragma once
#include <memory>
#include "IEngine.h"
#include "SceneManager.h"

class Game {
public:
	Game();
	~Game();

	void Run();

private:
	std::unique_ptr<IEngine> engine_ = nullptr;
	std::unique_ptr<SceneManager> sceneManager_ = nullptr;
};