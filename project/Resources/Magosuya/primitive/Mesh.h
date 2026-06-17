#pragma once
#include "struct.h"
#include "LineRenderData.h"
#include "CubeRenderData.h"

namespace Mesh {
	/// <summary>
	/// 線を描画
	/// </summary>
	/// <param name="data">線の頂点と色</param>
	/// <param name="vp">vp行列</param>
	void DrawLine (
		float posX1, float posY1, float posZ1, float posX2, float posY2, float posZ2,
		Vector4 color, const Matrix4x4& vp
	);

	/// <summary>
	/// キューブを描画
	/// </summary>
	/// <param name="data">キューブの頂点と色</param>
	/// <param name="vp">vp行列</param>
	void DrawCube (CubeData* data, const Matrix4x4& vp);

	void AllDrawing ();
}