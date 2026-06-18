#pragma once
#include <Windows.h>
#include <memory>
#include "Engine.h"
#include "SceneManager.h"

class Game {
public:
	Game();
	~Game();

	void Run();

private:
	std::unique_ptr<Engine> engine_ = nullptr;
	//std::unique_ptr<SceneManager> sceneManager_ = nullptr;
};