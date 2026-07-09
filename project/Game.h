#pragma once
#include <memory>
#include "SceneManager.h"

namespace MyEngine::LowLevel {
	class Engine;
}

class Game {
public:
	Game();
	~Game();

	void Run();

private:
	std::unique_ptr<MyEngine::LowLevel::Engine> engine_ = nullptr;
	std::unique_ptr<SceneManager> sceneManager_ = nullptr;
};