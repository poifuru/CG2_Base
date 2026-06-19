#include <cassert>
#include "Animation.h"
#include "MathFunction.h"
namespace AnimationFunc {

	Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time) {
		//***特殊なケースを除外する***//
		// キーが無いものは返す値がわからないのでassert
		assert(!keyframes.empty());
		// キーが1つか、時刻がキーフレーム前なら最初の値とする
		if(keyframes.size() == 1 || time <= keyframes[0].time) {
			return keyframes[0].value;
		}

		//ここから実際に計算
		for(size_t index = 0; index < keyframes.size() - 1; ++index) {
			size_t newIndex = index + 1;
			// indexとnewIndexの2つのkeyframeを取得して範囲内に時刻があるかを判定
			if(keyframes[index].time <= time && time <= keyframes[newIndex].time) {
				//範囲内を補間する
				float t = (time - keyframes[index].time) / (keyframes[newIndex].time - keyframes[index].time);
				return Math::Lerp(keyframes[index].value, keyframes[newIndex].value, t);
			}
		}
		//	ここまで来た場合は一番後の時刻よりも後ろなので最後の値を返すことにする
		return (*keyframes.rbegin()).value;
	}

	Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time) {
		//***特殊なケースを除外する***//
		// キーが無いものは返す値がわからないのでassert
		assert(!keyframes.empty());
		// キーが1つか、時刻がキーフレーム前なら最初の値とする
		if(keyframes.size() == 1 || time <= keyframes[0].time) {
			return keyframes[0].value;
		}

		//ここから実際に計算
		for(size_t index = 0; index < keyframes.size() - 1; ++index) {
			size_t newIndex = index + 1;
			// indexとnewIndexの2つのkeyframeを取得して範囲内に時刻があるかを判定
			if(keyframes[index].time <= time && time <= keyframes[newIndex].time) {
				//範囲内を補間する
				float t = (time - keyframes[index].time) / (keyframes[newIndex].time - keyframes[index].time);
				return Math::Lerp(keyframes[index].value, keyframes[newIndex].value, t);
			}
		}
		//	ここまで来た場合は一番後の時刻よりも後ろなので最後の値を返すことにする
		return (*keyframes.rbegin()).value;
	}

}