#include "PCH.h"
#include "ColorUtils.h"

float ColorUtils::GammaToLinear(float val) {
	if (val <= 0.04045f) {
		return val / 12.92f;
	}
	return std::pow((val + 0.055f) / 1.055f, 2.4f);
}

Vector4 ColorUtils::ToLinear(const Vector4& srgbColor) {
	return Vector4(
		ColorUtils::GammaToLinear(srgbColor.x),
		ColorUtils::GammaToLinear(srgbColor.y),
		ColorUtils::GammaToLinear(srgbColor.z),
		srgbColor.w // Alpha値は変換しない
	);
}