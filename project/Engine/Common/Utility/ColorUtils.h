#pragma once

namespace ColorUtils {
	/// <summary>
	/// 1要素のsRGB->Linear変換
	/// </summary>
	/// <param name="val"></param>
	/// <returns></returns>
	float GammaToLinear(float val);

	/// <summary>
	/// Vector3のsRGB->Linear変換
	/// </summary>
	/// <param name="srgbColor"></param>
	/// <returns></returns>
	Vector3 ToLinear(const Vector3& srgbColor);

	/// <summary>
	/// Vector4のsRGB->Linear変換
	/// </summary>
	/// <param name="srgbColor"></param>
	/// <returns></returns>
	Vector4 ToLinear(const Vector4& srgbColor);
}