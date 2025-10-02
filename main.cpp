#include <Windows.h>
#include "engine/EngineCore/DxCommon.h"
#include "Utility/Shape/Shape.h"
#include "header/struct.h"

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain (HINSTANCE, HINSTANCE, LPSTR, int) {
	MSG msg{};

	DxCommon dxCommon;
	dxCommon.Initialize ();

	//三角形のリソース
	ShapeData triangle{};

	/*メインループ！！！！！！！！！*/
	//ウィンドウの×ボタンが押されるまでループ
	while (msg.message != WM_QUIT) {
		//Windowにメッセージが来てたら最優先で処理させる
		if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}

		dxCommon.BeginFrame ();

		dxCommon.shape.DrawTriangle ();

		dxCommon.EndFrame ();
	}
	dxCommon.Finalize ();
	return 0;
};