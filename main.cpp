#include <Windows.h>
#include "engine/EngineCore/DxCommon.h"
#include "Utility/Shape/Shape.h"
#include "header/struct.h"

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain (HINSTANCE, HINSTANCE, LPSTR, int) {
	MSG msg{};

	DxCommon dxCommon;
	dxCommon.Initialize ();

	//頂点データ
	VertexData* vertexData[3] = {};
	*vertexData[0] = {
		{0.0f, 0.5f, 0.0f, 1.0f },
		{0.0f, 0.0f},
		{0.0f, 0.0f, -1.0f},
	};
	*vertexData[1] = {
		{0.5f, -0.5f, 0.0f, 1.0f },
		{0.0f, 0.0f},
		{0.0f, 0.0f, -1.0f},
	};
	*vertexData[2] = {
		{-0.5f, -0.5f, 0.0f, 1.0f },
		{0.0f, 0.0f},
		{0.0f, 0.0f, -1.0f},
	};

	//マテリアルデータ
	Material * materialData{};
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = true;
	materialData->uvTranform = MakeIdentity4x4 ();

	//トランスフォーム
	Transform transformTriangle = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	//UVトランスフォーム
	Transform uvTransform = { { 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f } };

	//カメラ
	Transform cameraTransform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -10.0f} };
	Matrix4x4 cameraMatrix = {};
	Matrix4x4 viewMatrix = {};
	Matrix4x4 projectionMatrix = {};

	//三角形のリソース
	ShapeData triangle{};
	for (int i = 0; i < 3; i++) {
		triangle.vertexData.push_back (*vertexData[i]);
	}
	triangle.materialData = *materialData;
	triangle.matrixData

		/*メインループ！！！！！！！！！*/
		//ウィンドウの×ボタンが押されるまでループ
		while (msg.message != WM_QUIT) {
			//Windowにメッセージが来てたら最優先で処理させる
			if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage (&msg);
				DispatchMessage (&msg);
			}

			dxCommon.BeginFrame ();

			//カメラ
			cameraMatrix = MakeAffineMatrix (cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
			viewMatrix = Inverse (cameraMatrix);
			projectionMatrix = MakePerspectiveFOVMatrix (0.45f, float (1280) / float (720), 0.1f, 100.0f);

			dxCommon.shape.DrawTriangle (&triangle, &dxCommon.GetTextureHandle ());

			dxCommon.EndFrame ();
		}
	dxCommon.Finalize ();
	return 0;
};