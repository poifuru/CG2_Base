#pragma once

namespace ColorUtils {
	/// <summary>
	/// 1要素のsRGB
	/// </summary>
	/// <param name="val"></param>
	/// <returns></returns>
	float GammaToLinear(float val);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="srgbColor"></param>
	/// <returns></returns>
	Vector4 ToLinear(const Vector4& srgbColor);
}