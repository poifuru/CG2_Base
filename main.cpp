#include <Windows.h>
#include "engine/EngineCore/DxCommon.h"

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain (HINSTANCE, HINSTANCE, LPSTR, int) {
	MSG msg{};

	DxCommon dxCommon;
	dxCommon.Initialize ();
	/*メインループ！！！！！！！！！*/
	//ウィンドウの×ボタンが押されるまでループ
	while (msg.message != WM_QUIT) {
		//Windowにメッセージが来てたら最優先で処理させる
		if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}

		dxCommon.BeginFrame ();

		dxCommon.DrawTriangle ();

		dxCommon.EndFrame ();
	}
	dxCommon.Finalize ();
	return 0;
};