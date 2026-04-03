#include "Game.h"

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain (_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
	std::unique_ptr<Game> game = std::make_unique<Game>();

	game->Run();

	return 0;
};